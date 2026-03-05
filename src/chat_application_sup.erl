%%%-------------------------------------------------------------------
%% @doc chat_application top level supervisor.
%% @end
%%%-------------------------------------------------------------------

-module(chat_application_sup).

-behaviour(supervisor).

-export([start_link/0]).

-export([init/1]).

-define(SERVER, ?MODULE).

start_link() ->
    supervisor:start_link({local, ?SERVER}, ?MODULE, []).

%% sup_flags() = #{strategy => strategy(),         % optional
%%                 intensity => non_neg_integer(), % optional
%%                 period => pos_integer()}        % optional
%% child_spec() = #{id => child_id(),       % mandatory
%%                  start => mfargs(),      % mandatory
%%                  restart => restart(),   % optional
%%                  shutdown => shutdown(), % optional
%%                  type => worker(),       % optional
%%                  modules => modules()}   % optional
init([]) ->
    SupFlags = #{
        strategy => one_for_all,
        intensity => 0,
        period => 1
    },
    ChildSpecs = [#{id => mnesia_database, start => {mnesia_database, start_link, []}, restart => permanent, shutdown => 2000, type => worker, modules => [mnesia_database]},
                  #{id => chat_server, start => {chat_app_server, start_link, []}, restart => permanent, shutdown => 2000, type => worker, modules => [chat_app_server]}],
    {ok, {SupFlags, ChildSpecs}}.

%% internal functions
