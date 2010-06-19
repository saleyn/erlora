%%%------------------------------------------------------------------------
%%% File: $Id$
%%%------------------------------------------------------------------------
%%% @doc This module implements client interface to a port program
%%%      communicating with an Oracle 10g client.
%%%
%%% @author  Serge Aleynikov <saleyn@gmail.com>
%%% @version $Revision$
%%%          $Date$
%%% @end
%%% Created: 10-Aug-2006 by Serge Aleynikov <saleyn@gmail.com>
%%%------------------------------------------------------------------------
%%% @type date_time() = {{Y, M, D}, {H, Mi, S}}.
%%%          Data type storing date and time value.
%%% @type timeout() = infinity | integer().
%%%          Timeout value of a function call.
%%%          If the value is inteter() it must be > 0.
%%% @type ora_error() = {Class, ErrCode::integer(), Err::string()}.
%%%             Class = encode | decode | occi | ora.
%%%          Error types returned by an Oracle port program.
%%%------------------------------------------------------------------------
-module(ora).
-author('saleyn@gmail.com').

% External API exports
-export([start/0, start/1, connect/2, disconnect/1, commit/1, rollback/1,
         describe_table/2, execute_sql/4, select_sql/4, free_sql/2, prepare_plsql/4,
         execute_plsql/3]).

-behaviour(gen_server).

% Internal gen_server exports
-export([
     start_link/2,
     init/1,
     handle_call/3,
     handle_cast/2,
     handle_info/2,
     code_change/3,
     terminate/2
    ]).

-include("ora_internal.hrl").

%% Internal state
-record(state, {
        erlang_port,                 % The port to the c-program
        reply_to,                    % gen_server From parameter
		owner,                       % Pid of the connection owner
        super,                       % Pid of the supervisor
		result_set = undefined,      % exists | undefined
		auto_commit_mode = on,       % on | off
        %% connected | disconnected
        status = disconnected,
		%% For timeout handling
        pending_request
       }).

%%%--------------------------------------------------------------------------
%%% API
%%%--------------------------------------------------------------------------

%% Start/Stop functionality is handled by connect/2 and disconnect/1

start() ->
    start([]).

start(Args) ->
    % Since the simple_one_for_one restart strategy is used by the application's
    % supervisor, we must pass a list of arguments as the ChildSpec
    NewArgs = [{owner, self()} | Args],
    supervisor:start_child(ora_sup, [ NewArgs ]).

%%-------------------------------------------------------------------------
%% @spec connect(ConnectionStr::string(), Options) -> Result
%%          Options = [Option]
%%          Option  = {autocommit,       boolean()} |
%%                    {max_rows,         integer()}  |
%%                    {query_cache_size, integer()}
%%          Result  = {ok, ConnectionPid::pid()} | {error, Reason::ora_error()}
%%
%% @doc Spawns an erlang control process that will open a port
%%      to a c-process that uses the ORACLE OCCI API to open a connection
%%      to the database.  ``ConnectionStr'' can contain
%%      "user/password@database" connection string.  ``max_rows'' option limits
%%      the max number of elements in the list of rows returned by a select
%%      statement.  ``query_cache_size'' controls the size of the driver's cache
%%      that stores prepared query statements.  Its default value is 50.
%%      The port program can be started with the "-d" flag by
%%      specifying {mod, {ora, [{debug, true}]}} option in the ora.app file.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%%   ( {connect, ConnectionStr, Options} ) -> ok | {error, Reason::ora_error()}
%%       ConnectStr = string()
%%       Options    = [Option]
%%       Option     = {autocommit,       true|false} |
%%                    {max_rows,         integer()}  |
%%                    {query_cache_size, integer()}
%% '''
%% @end
%%-------------------------------------------------------------------------
connect(ConnectionStr, Options) when list(ConnectionStr), list(Options) ->
    %% Start of the ora application should really be handled by the
    %% application using odbc.
    case application:start(ora) of
    {error, {already_started,ora}} ->
        ok;
    ok ->
        error_logger:info_report("The ora application was not started."
                                 " Has now been started as a temporary"
                                 " application.")
    end,

    %% Spawn the erlang control process and issue a connect command.
    case supervisor:start_child(ora_sup, [ [{owner, self()}] ]) of
    {ok, Pid} ->
        case gen_server:call(Pid, {connect, ConnectionStr, Options}, 15000) of
        ok ->
            {ok, Pid};
        {error, Reason} ->
            {error, Reason}
        end;
    {error, Reason} ->
        {error, Reason}
    end.

%%--------------------------------------------------------------------------
%% @spec disconnect(ConnectionReferense::pid()) -> ok | {error, Reason}
%%
%% @doc Disconnects from the database and terminates both the erlang
%%      control process and the database handling c-process.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%%   ( disconnect ) -> ok | {error, Reason::ora_reason()}
%% '''
%% @end
%%--------------------------------------------------------------------------
disconnect(ConnectionReference) when is_pid(ConnectionReference) ->
    gen_server:cast(ConnectionReference, disconnect).

%%--------------------------------------------------------------------------
%% @spec rollback(ConnectionReference::pid()) -> ok | {error,Reason}
%% @doc Rolls back an open transaction.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%%   ( rollback ) -> ok | {error, Reason::ora_reason()}
%% '''
%% @see commit/3
%% @end
%%--------------------------------------------------------------------------
rollback(ConnectionReference) when is_pid(ConnectionReference) ->
    gen_server:call(ConnectionReference, rollback, ?DEFAULT_TIMEOUT).

%%--------------------------------------------------------------------------
%% @spec commit(ConnectionReference::pid()) ->
%%            ok | {error,Reason}
%%
%% @doc Commits or rollbacks a transaction. Needed on connections
%%      where automatic commit is turned off.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%%   ( commit ) -> ok | {error, Reason::ora_reason()}
%% '''
%% @end
%%--------------------------------------------------------------------------
commit(ConnectionReference) ->
    gen_server:call(ConnectionReference, commit, ?DEFAULT_TIMEOUT).

%%--------------------------------------------------------------------------
%% @spec describe_table(ConnectionReference::pid(), Table::string()) ->
%%           {ok, FieldNames} | {error, Reason}
%%       Table      = string()
%%       Fields     = [{FieldName::string(), Datatype, Size::integer()}]
%%       Datatype   = integer | float | string | date
%%       Reason     = ora_error()
%%
%% @doc Queries the database to find out the datatypes of the Table.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%%   ( {describe, Table} ) ->
%%           {ok, Fields} | {error, Reason}
%%
%% Note: Rows will be an empty list, because the select statement
%%       for the describe_table command will not return any data records.
%% '''
%% @end
%%--------------------------------------------------------------------------
describe_table(ConnectionReference, Table)
  when is_pid(ConnectionReference), is_list(Table) ->
    gen_server:call(ConnectionReference, {describe, Table}, ?DEFAULT_TIMEOUT).

%%--------------------------------------------------------------------------
%% @spec select_sql(ConnectionReference::pid(), QueryID, Query, BindVars) -> Result
%%          QueryID    = atom()
%%          Query      = string() | binary()
%%          BindVars   = [BindVar]
%%          BindVar    = integer() | float() | string() | date_time()
%%          Result     = {ok, FieldNames, Rows, Continue} |
%%                       {error, Reason::ora_error()}
%%          Continue   = eof | continue
%%          FieldNames = [FieldName::string()] | []
%%          Rows       = [Row::tuple()]
%%
%% @doc Executes a SELECT SQL query. The result set is returned (up to N
%%      defined by the {max_rows, N} connect
%%      option, in which case select_next/2 can be used to fetch next N records),
%%      otherwise the number of affected rows is returned.
%%      The Query should be an SQL string. QueryID is an atom that causes the
%%      driver to cache the prepared SQL statement associated with QueryID.
%%      That internal cache size is controlled by the session's ``query_cache_size''
%%      parameter.  When ``select_sql'' is used to fetch large datasets that
%%      return more than ``max_rows'' records, ``{ok, _, _, continue}'' tuple is returned
%%      indicating that there is more data in the cursor, otherwise ``{ok, _, _, eof}''
%%      is returned.  In the former case the next record list can be fetched using
%%      the same ``select_sql'' function call as the driver remembers the state
%%      of the open cursor associated with the QueryID.  Also successive calls
%%      of this function will return an empty list of FieldNames, since the field names
%%      can be obtained from the first call of the function.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%% ( {select_sql, QueryID, Query, BindVars} ) ->
%%         {ok, FieldNames, Rows, eof}      |   % For a select query with resultset < max_rows
%%         {ok, FieldNames, Rows, continue} |   % For a select query with remaining records in a cursor
%%         {ok, OutParams::list()}          |   % For plsql block
%%         {error, Reason}
%% '''
%% @end
%%--------------------------------------------------------------------------
select_sql(ConnectionReference, QueryID, Query, BindVars)
  when is_pid(ConnectionReference), is_atom(QueryID), is_list(Query) ->
    gen_server:call(ConnectionReference, {select_sql, QueryID, Query, BindVars}, infinity).

%%--------------------------------------------------------------------------
%% @spec execute_sql(ConnectionReference::pid(), QueryID, Query, BindVars) -> Result
%%          QueryID    = atom()
%%          Query      = string() | binary()
%%          BindVars   = [BindVar]
%%          BindVar    = integer() | float() | string() | date_time()
%%          Result     = {ok, NRows::integer()} |
%%                       {error, Reason::ora_error()}
%%          Continue   = eof | continue
%%          FieldNames = [FieldName::string()] | []
%%          Rows       = [Row::tuple()]
%%
%% @doc Executes an UPDATE/DELETE/INSERT SQL query.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%% ( {execute_sql, QueryID, Query, BindVars} ) ->
%%         {ok, FieldNames, Rows, eof}      |   % For a select query with resultset < max_rows
%%         {ok, FieldNames, Rows, continue} |   % For a select query with remaining records in a cursor
%%         {ok, NRows::integer()}           |   % For insert, update, delete query
%%         {ok, OutParams::list()}          |   % For plsql block
%%         {error, Reason}
%% '''
%% @see select_sql/4
%% @end
%%--------------------------------------------------------------------------
execute_sql(ConnectionReference, QueryID, Query, BindVars)
  when is_pid(ConnectionReference), is_atom(QueryID), is_list(Query) ->
    gen_server:call(ConnectionReference, {execute_sql, QueryID, Query, BindVars}, infinity).

%%--------------------------------------------------------------------------
%% @spec free_sql(ConnectionReference::pid(), QueryID::atom()) -> ok
%%
%% @doc Releases resources allocated by QueryID associated with a former
%%      call to ``execute_sql/4''.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%% ( {free_sql, QueryID} ) -> ok
%% '''
%% @end
%%--------------------------------------------------------------------------
free_sql(ConnectionReference, QueryID)
  when is_pid(ConnectionReference), is_atom(QueryID) ->
    gen_server:call(ConnectionReference, {free_sql, QueryID}, infinity).

%%--------------------------------------------------------------------------
%% @spec prepare_plsql(ConnectionReference::pid(), QueryID::atom(),
%%                     Query, OutVars) -> ok | {error, Reason}
%%          Query     = string() | binary()
%%          OutVars   = {out, ParamNum::integer(), ParamType}
%%          ParamType = integer | float | date | {string, Size::integer()}
%%
%% @doc Prepares a PL/SQL block for subsequent execution using
%%      ``execute_plsql/3''.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%% ( {prepare_plsql, QueryID, Query, OutVars} ) -> ok | {error, Reason}
%% '''
%% @end
%%--------------------------------------------------------------------------
prepare_plsql(ConnectionReference, QueryID, Query, OutVars)
  when is_pid(ConnectionReference), is_atom(QueryID),
       is_list(Query) orelse is_binary(Query), is_list(OutVars) ->
    Args = {prepare_plsql, QueryID, Query, OutVars},
    gen_server:call(ConnectionReference, Args, infinity).

%%--------------------------------------------------------------------------
%% @spec execute_plsql(ConnectionReference::pid(), QueryID::atom(),
%%                     BindVars) -> {ok, OutValues} | {error, Reason}
%%          BindVars  = {in, ParamNum::integer(), Value}
%%          Value     = integer() | float() | date_time() | string()
%%          OutValues = [{OutParamNum::integer(), Value}]
%%
%% @doc Executes a PL/SQL block previously prepared using ``prepare_plsql/4''.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%% ( {execute_plsql, QueryID, BindVars} ) ->
%%      {ok, OutValues} | {error, Reason}
%% '''
%% @end
%%--------------------------------------------------------------------------
execute_plsql(ConnectionReference, QueryID, BindVars)
  when is_pid(ConnectionReference), is_atom(QueryID), is_list(BindVars) ->
    gen_server:call(ConnectionReference, {run_plsql, QueryID, BindVars}, infinity);

%%--------------------------------------------------------------------------
%% @spec execute_plsql(ConnectionReference::pid(), Query,
%%                     BindVars) -> {ok, OutValues} | {error, Reason}
%%          BindVars  = {InOut, ParamNum::integer(), ValueType}
%%          InOut     = in | out | inout
%%          ValueType = integer() | float() | date_time() | string() |
%%                      integer | float | date | {string, Size::integer()}
%%          OutValues = [{OutParamNum::integer(), Value}]
%%
%% @doc Executes a PL/SQL block.
%% ```
%% Erlang -> C -> Erlang port matshaling spec:
%% ( {run_plsql, QueryID, BindVars} ) ->
%%      {ok, OutValues} | {error, Reason}
%% '''
%% @end
%%--------------------------------------------------------------------------
execute_plsql(ConnectionReference, Query, BindVars)
  when is_pid(ConnectionReference), is_list(Query) orelse is_binary(Query),
       is_list(BindVars) ->
    gen_server:call(ConnectionReference, {exec_plsql, Query, BindVars}, infinity).

%%--------------------------------------------------------------------------
%% @spec start_link(StartArgs::list(), Args::list()) -> {ok, Pid} | {error, Reason}
%%
%% @doc Callback function for the ora supervisor. It is called
%%      when connect/2 calls supervisor:start_child/2 to start an
%%      instance of the erlang ora control process. ``StartArgs'' are
%%      arguments passed from the *.app ``{mod, StartArgs}'' spec, and
%%      ``Args'' are arguments passed to the supervisor:start_link/2
%%      call
%% @end
%% @private
%%--------------------------------------------------------------------------
start_link(StartArgs, Args) when is_list(StartArgs), is_list(Args) ->
    gen_server:start_link(?MODULE, [StartArgs ++ Args], []).

%%%%========================================================================
%%% Callback functions from gen_server
%%%========================================================================

%%-------------------------------------------------------------------------
%% init([Parent, Args]) -> {ok, State} | {ok, State, Timeout} | {stop, Reason}
%% Description: Initiates the erlang process that manages the connection
%%              and starts the port-program that use the odbc driver
%%		to communicate with the database.
%% @private
%%-------------------------------------------------------------------------
init([Args]) ->
    process_flag(trap_exit, true),
    case catch begin
        Parent     = proplists:get_value(owner, Args),
        Supervisor = proplists:get_value(sup,   Args),
        Debug      = proplists:get_value(debug, Args, false) orelse
                     case application:get_env(ora, debug) of
                     {ok, Val} -> Val;
                     undefined -> false
                     end,

        Priv = case code:priv_dir(ora) of
               L when is_list(L) ->
                   L;
               {error, bad_name} ->
                   exit("Cannot determine location of the 'priv' dir. Include '-pa ../../ora/ebin' in the path")
               end,

        Dir = filename:nativename(Priv),

        %% Start the port program (a c program) that utilizes the odbc driver
        case os:find_executable(?SERVERPROG, Dir) of
        FileName when list(FileName) ->
            FN = case Debug of
                 true  -> FileName ++ " -d";  % set the debug flag
                 false -> FileName
                 end,
            case catch open_port({spawn, FN},
                                 [{packet, ?LENGTH_INDICATOR_SIZE}, binary, exit_status]) of
            Port when is_port(Port) ->
                erlang:monitor(process, Parent),
                State = #state{erlang_port=Port, owner=Parent, super=Supervisor},
                {ok, State};
            {'EXIT', Reason} ->
                Str = io_lib:format("Error starting port \"~s\": ~p~n", [FN, Reason]),
                {stop, lists:flatten(Str)}
            end;
        false ->
            {stop, {'Port Program Not Found', ?SERVERPROG}}
        end
    end of
    {ok, Pid} ->
        {ok, Pid};
    {'EXIT', Why} ->
        {stop, Why};
    {stop, Why} ->
        {stop, Why}
    end.

%%--------------------------------------------------------------------------
%% @spec handle_call(Request, From, State) ->
%%          {reply, Reply, State} |
%%          {reply, Reply, State, Timeout} |
%%          {noreply, State}               |
%%          {noreply, State, Timeout}      |
%%          {stop, Reason, Reply, State}   |
%%          {stop, Reason, Reply, State}
%% @doc Handle incoming requests. Only requests from the process
%%      that created the connection are allowed in order to preserve
%%      the semantics of result sets.
%%      Note: The order of the function clauses is significant.
%% @private
%% @end
%%--------------------------------------------------------------------------

%% This is an attempt to call a server function when a reply hasn't been
%% received from the port program
handle_call(_Term, _From, #state{reply_to = {_Pid, _Ref}} = State) ->
    {reply, {error, server_busy}, State};

%% This is a request to connect to Oracle
handle_call({connect, _, _} = Msg, From, #state{erlang_port=Port, status=disconnected} = State) ->
    erlang:port_command(Port, term_to_binary(Msg)),
    {noreply, State#state{reply_to = From, status = connecting}};

%% This is some other command sent before an Oracle connection is established
handle_call(_Command, _From, #state{status = disconnected} = State) ->
    {reply, {error, session_not_connected}, State};

%% Any other command
handle_call(Msg, From, #state{erlang_port = Port} = State) ->
    erlang:port_command(Port, term_to_binary(Msg)),
    {noreply, State#state{reply_to = From}}.

%%--------------------------------------------------------------------------
%% handle_cast(Request, State) -> {noreply, State} |
%%                                {noreply, State, Timeout} |
%%                                {stop, Reason, State}
%% Description: Handles cast messages.
%% Note: The order of the function clauses is significant.
%% @private
%%-------------------------------------------------------------------------

%% Contrary to other commands, disconnect is asynchronous
handle_cast(disconnect, #state{erlang_port = Port} = State) ->
    erlang:port_command(Port, term_to_binary(disconnect)),
    {noreply, State#state{status = disconnected}};

%% Catch all - This can only happen if the application programmer writes
%% really bad code that violates the API.
handle_cast(Msg, State) ->
    {stop, {'API_violation_connection_closed', Msg}, State}.

%%--------------------------------------------------------------------------
%% handle_info(Msg, State) -> {noreply, State} | {noreply, State, Timeout} |
%%			      {stop, Reason, State}
%% Description: Handles timouts, replys from the port-program and EXIT and
%%		down messages.
%% Note: The order of the function clauses is significant.
%% @private
%%--------------------------------------------------------------------------

%% We got a reply from a previously sent command to the Port.  Relay it to the caller.
handle_info({Port, {data, Data}},
            #state{erlang_port=Port, reply_to=From, status=Status} = State)
  when is_binary(Data) ->
    Reply = binary_to_term(Data),
    case {Status, Reply} of
    {connecting, ok} ->
        %% This message is a reply from a prior connection command sent to
        %% the port program. Set the status to connected, and wait for a next command.
        gen_server:reply(From, Reply),
        {noreply, State#state{reply_to=undefined, status=connected}};
    {connecting, {error, Reason}} ->
        %% This is a result of a failed connection attempt. Crash the server
        {stop, Reason, State#state{status=disconnected}};
    {connected, _} ->
        gen_server:reply(From, Reply),
        {noreply, State#state{reply_to=undefined}};
    {disconnected, _} ->
        catch erlang:port_close(Port),
        {stop, normal, State}
    end;

handle_info({Port, {exit_status, Status}},
	    State = #state{erlang_port = Port}) ->
    % ?ERROR("Oracle port exited with reason: ~w~n", [?PORT_EXIT_REASON(Status)]),
    case Status of
    ?EXIT_SUCCESS ->
        {stop, normal, State};
    _ ->
        {stop, {port_exit, ?PORT_EXIT_REASON(Status)}, State}
    end;

handle_info({'EXIT', Pid, Reason}, State = #state{erlang_port=Port, super=Sup})
  when Pid == Port; Pid == Sup ->
    {stop, Reason, State};

%%% If the owning process dies there is no reson to go on
handle_info({'DOWN', _Ref, _Type, Owner, _Reason}, #state{owner=Owner} = State) ->
    {stop, normal, State#state{reply_to = undefined}};

%---------------------------------------------------------------------------
%% Catch all - throws away unknown messages (This could happen by "accident"
%% so we do not want to crash, but we make a log entry as it is an
%% unwanted behaviour.)
handle_info(Info, State) ->
    error_logger:error_report("ORA: received unexpected info: ~p~n", [Info]),
    {noreply, State}.

%%--------------------------------------------------------------------------
%% terminate/2 and code_change/3
%% @private
%%--------------------------------------------------------------------------

terminate(_Reason, #state{erlang_port = Port, reply_to = undefined}) ->
    catch port_close(Port),
    ok;

terminate(Reason, State = #state{reply_to = From}) ->
    gen_server:reply(From, {error, connection_closed}),
    terminate(Reason, State#state{reply_to = undefined}).

%---------------------------------------------------------------------------
%% @private
%---------------------------------------------------------------------------
code_change(_Vsn, State, _Extra) ->
    {ok, State}.


%%%========================================================================
%%% Internal functions
%%%========================================================================
