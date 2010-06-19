%%%------------------------------------------------------------------------
%%% File: $Id$
%%%------------------------------------------------------------------------
%%% @doc The main application file of ORA.
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

-module(ora_app).

%% Application callbacks
-export([start/2, stop/1]).

%% Supervisor callback
-export([init/1]).

-behaviour(application).

%% @hidden
start(_Type, Args) ->
    supervisor:start_link({local, ora_sup}, ?MODULE, [Args]).

%% @hidden
stop(_State) ->
    ok.

%% @hidden
init([Args]) ->
    RestartStrategy = {simple_one_for_one, _MaxR = 0, _MaxT = 3600},
    StartFunc       = {ora, start_link, [ [{sup, self()} | Args] ]},
    Restart         = temporary, % E.g. should not be restarted
    Shutdown        = 7000,
    Modules         = [ora],
    Type            = worker,
    % Because simple_one_for_one restart is chosed the ChildSpec must contain
    % *one* specification, which will not be started at startup, but will be
    % started dynamically using supervisor:start_child/2
    ChildSpec       = [{ora, StartFunc, Restart, Shutdown, Type, Modules}],
    {ok, {RestartStrategy, ChildSpec}}.
