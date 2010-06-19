%%%------------------------------------------------------------------------
%%% File: $Id$
%%%------------------------------------------------------------------------
%%% @author  Serge Aleynikov <saleyn@gmail.com>
%%% @end
%%% Created: 10-Aug-2006 by Serge Aleynikov <saleyn@gmail.com>
%%%------------------------------------------------------------------------

%% Path to the c-program.
-define(SERVERDIR, filename:nativename(filename:join(code:priv_dir(ora), "bin"))).

%% Name of the C program
-define(SERVERPROG, "oraserver").

%% Constats defining the command protocol between the erlang control
%% process and the port program. These constants must also be defined
%% in the same way in the port program.

-define(LENGTH_INDICATOR_SIZE,  4).
-define(INT_VALUE,              1).
-define(STR_VALUE,              2).
-define(ON,                     1).
-define(OFF,                    2).
-define(DUMMY_OFFSET,           0).


%% Types of parameters given to param_query
-define(USER_SMALL_INT, 1).
-define(USER_INT,       2).
-define(USER_DECIMAL,   3).
-define(USER_NUMERIC,   4).
-define(USER_CHAR,      5).
-define(USER_VARCHAR,   6).
-define(USER_FLOAT,     7).
-define(USER_REAL,      8).
-define(USER_DOUBLE,    9).
-define(USER_BOOLEAN,  10).
-define(USER_TINY_INT, 11).

%% EXIT CODES
-define(EXIT_SUCCESS,            0). % As defined in c iso_stdlib
-define(EXIT_FAILURE,            1). % As defined in c iso_stdlib
-define(EXIT_ALLOC,              2).
-define(EXIT_ENV,                3).
-define(EXIT_CONNECTION,         4).
-define(EXIT_FREE,               5).
-define(EXIT_STDIN_HEADER,       6).
-define(EXIT_STDIN_BODY,         7).
-define(EXIT_BIN,                8).
-define(EXIT_THREAD,             9).
-define(EXIT_PARAM_ARRAY,        10).
-define(EXIT_OLD_WINSOCK,        11).
-define(EXIT_SOCKET_CONNECT,     12).
-define(EXIT_SOCKET_SEND_HEADER, 13).
-define(EXIT_SOCKET_SEND_BODY,	 14).
-define(EXIT_SOCKET_RECV_MSGSIZE,15).
-define(EXIT_SOCKET_SEND_MSGSIZE,16).
-define(EXIT_SOCKET_RECV_HEADER, 17).
-define(EXIT_SOCKET_RECV_BODY,   18).
-define(EXIT_COLS,               19).
-define(EXIT_ROWS,               20).
-define(EXIT_DESC,               21).
-define(EXIT_BIND,               22).
-define(EXIT_DRIVER_INFO,        23).

%% Misc constants
-define(DEFAULT_TIMEOUT, infinity).
-define(STR_TERMINATOR, 0).
-define(MAX_SEQ_TIMEOUTS, 10).

%% Handling of C exit codes
-define(ENCODE_EXIT_FUN,
	(fun(?EXIT_SUCCESS)             -> normal_exit;
	    (?EXIT_FAILURE)             -> abnormal_exit;
	    (?EXIT_ALLOC)               -> memory_allocation_failed;
	    (?EXIT_ENV)                 -> setting_of_environment_attributes_failed;
	    (?EXIT_CONNECTION)          -> setting_of_connection_attributes_faild;
	    (?EXIT_FREE)                -> freeing_of_memory_failed;
	    (?EXIT_STDIN_HEADER)        -> receiving_port_message_header_failed;
	    (?EXIT_STDIN_BODY)          -> receiving_port_message_body_failed;
	    (?EXIT_BIN)                 -> retrieving_of_binary_data_failed;
	    (?EXIT_THREAD)              -> failed_to_create_thread;
	    (?EXIT_PARAM_ARRAY)         -> does_not_support_param_arrays;
	    (?EXIT_OLD_WINSOCK)         -> too_old_verion_of_winsock;
	    (?EXIT_SOCKET_CONNECT)      -> socket_connect_failed;
	    (?EXIT_SOCKET_SEND_HEADER)  -> socket_send_message_header_failed;
	    (?EXIT_SOCKET_SEND_BODY)    -> socket_send_message_body_failed;
	    (?EXIT_SOCKET_RECV_MSGSIZE) -> socket_received_too_large_message;
	    (?EXIT_SOCKET_SEND_MSGSIZE) -> too_large_message_in_socket_send;
	    (?EXIT_SOCKET_RECV_HEADER)  -> socket_receive_message_header_failed;
	    (?EXIT_SOCKET_RECV_BODY)    -> socket_receive_message_body_failed;
	    (?EXIT_COLS)                -> could_not_access_column_count;
	    (?EXIT_ROWS)                -> could_not_access_row_count;
	    (?EXIT_DESC)                -> could_not_access_table_description;
	    (?EXIT_BIND)                -> could_not_bind_data_buffers;
	    (?EXIT_DRIVER_INFO)         -> collecting_of_driver_information_faild;
	    (_)                         -> killed
	 end)).

-define(PORT_EXIT_REASON(EXIT_STATUS),
	?ENCODE_EXIT_FUN(EXIT_STATUS)).
