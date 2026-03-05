-module(chat_app_server).
-export([start_link/0]).

start_link() ->
	Pid = spawn_link(fun() -> start_server() end),
	{ok, Pid}.

start_server() ->
	{ok,ListenSocket} = gen_tcp:listen(1234,[binary,{active,false},{reuseaddr,true}]),
	accept_connection_req(ListenSocket).

accept_connection_req(ListenSocket) ->
	{ok,NewSocket} = gen_tcp:accept(ListenSocket),
	spawn_link(start_communication,start,[NewSocket]),
	accept_connection_req(ListenSocket).
