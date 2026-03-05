%%%-------------------------------------------------------------------
%% @doc chat_application public API
%% @end
%%%-------------------------------------------------------------------

-module(chat_application_app).

-behaviour(application).

-export([start/2, stop/1]).

start(_StartType, _StartArgs) ->
    chat_application_sup:start_link().

stop(_State) ->
    ok.

%% internal functions
