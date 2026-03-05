-module(mnesia_database).
-export([start_link/0]).
-include("chat_app_records.hrl").
-include_lib("stdlib/include/qlc.hrl").
-behaviour(gen_server).
-export([init/1,handle_call/3,handle_cast/2,handle_info/2,terminate/2,code_change/3]).

start_link() ->
	gen_server:start_link({local,?MODULE},?MODULE,[],[]).
	
init([]) ->
	_ = mnesia:stop(),  %% Returns 'stopped' if running, 'ok' if not
	case mnesia:create_schema([node() | nodes()]) of
		ok -> ok;
		{error, {_, {already_exists, _}}} -> ok
	end,
	ok = mnesia:start(),
	RegisterOrDeregister = mnesia:create_table(registerorderegister,[{attributes,record_info(fields,registerorderegister)},{disc_copies,[node() | nodes()]},{type,set}]),
	check_table_creation_status(RegisterOrDeregister),
	%%LoginOrLogout = mnesia:create_table(loginorlogout,[{attributes,record_info(fields,loginorlogout)},{disc_copies,[node() | nodes()]},{type,set}]),
	%%check_table_creation_status(LoginOrLogout),
	{ok,[]}.
	
handle_call({register_the_user,UserName,Password},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName])) end),
	case Status of
		{atomic,[]} -> InsertDataToRegisterationTable = mnesia:transaction(fun() -> mnesia:write(#registerorderegister{username = UserName,password = Password}) end),
			  case InsertDataToRegisterationTable of
			  	{atomic,ok} -> {reply,"Registered Successfully",State};
			  	_ -> {reply,"Failed to Register",State}
			end;
		_ -> {reply,"User is already present",State}
	end;
handle_call({unregister_the_user,UserName,_Password},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName])) end),
	case Status of
		{atomic,[]} -> {reply,"Unknown User",State};
		{atomic,[_Record]} -> DeleteUserFromLoginTable = State -- [UserName], DeleteDataFromRegisterationTable = mnesia:transaction(fun() -> mnesia:delete({registerorderegister,UserName}) end),
			  case DeleteDataFromRegisterationTable of
			  	{atomic,ok} -> {reply,"Unregistered the user Successfully",DeleteUserFromLoginTable};
			  	_ -> {reply,"Failed to unregister",State}
			end
	end;
handle_call({login,UserName,Password},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName,X#registerorderegister.password =:= Password])) end),
	case Status of
		{atomic,[]} -> {reply,"Unknown User",State};
		{atomic,[_Record]} -> NewState = [UserName|State],{reply,"Successfully Logged In",NewState}
	end;
handle_call({logout,UserName,Password},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName,X#registerorderegister.password =:= Password])) end),
	case Status of
		{atomic,[]} -> {reply,"Unknown User",State};
		{atomic,[_Record]} -> NewState = State--[UserName],{reply,"Successfully Logged Out",NewState}
	end.
	
handle_cast(_message,State) ->
	{noreply,State}.

handle_info(_message,State) ->
	{noreply, State}.
terminate(_Reason,_State) ->
	ok.
code_change(_OldVsn, State, _Extra) ->
	{ok,State}.	
check_table_creation_status(TableResult) ->
	case TableResult of
        {atomic, ok} -> 
            io:format("Table created successfully!~n");
        {aborted, {already_exists, Chat_message}} -> 
            io:format("Table already exists, skipping...~n")
    end.
