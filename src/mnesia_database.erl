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
	ets:new(loginorlogout,[named_table,set,{keypos, #loginorlogout.username}]),
	%%LoginOrLogout = mnesia:create_table(loginorlogout,[{attributes,record_info(fields,loginorlogout)},{disc_copies,[node() | nodes()]},{type,set}]),
	%%check_table_creation_status(LoginOrLogout),
	{ok,loginorlogout}.
	
handle_call({register_the_user,UserName,Password,Socket_Info},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName])) end),
	case Status of
		{atomic,[]} -> InsertDataToRegisterationTable = mnesia:transaction(fun() -> mnesia:write(#registerorderegister{username = UserName,password = Password,socket_details = Socket_Info}) end),
			  case InsertDataToRegisterationTable of
			  	{atomic,ok} -> {reply,"Registered Successfully",State};
			  	_any -> io:format("here ~p ~n",[_any]),{reply,_any,State}
			end;
		_ -> {reply,"User is already present",State}
	end;
handle_call({unregister_the_user,UserName,Password},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName,X#registerorderegister.password =:= Password])) end),
	case Status of
		{atomic,[]} -> {reply,"Incorrect Username or Password-Failed to deregister",State};
		{atomic,[_Record]} -> DeleteUserFromLoginTable = ets:delete_object(loginorlogout,{loginorlogout,UserName}), DeleteDataFromRegisterationTable = mnesia:transaction(fun() -> mnesia:delete({registerorderegister,UserName}) end),
			  case DeleteDataFromRegisterationTable of
			  	{atomic,ok} -> {reply,"Unregistered the user Successfully",DeleteUserFromLoginTable};
			  	_ -> {reply,"Failed to unregister",State}
			end
	end;
handle_call({login,UserName,Password},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName,X#registerorderegister.password =:= Password])) end),
	case Status of
		{atomic,[]} -> {reply,"Incorrect Username or Password-Failed to LogIn",State};
		{atomic,[_Record]} -> CheckLoginStatus = ets:lookup(loginorlogout,UserName),
					case CheckLoginStatus of
						[] -> NewState = ets:insert(loginorlogout,#loginorlogout{username=UserName}),{reply,"Successfully Logged In",NewState};
						[_record] -> {reply,"User is Logged In, Logout before Login", State}
					end 
	end;
handle_call({logout,UserName,Password},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= UserName,X#registerorderegister.password =:= Password])) end),
	case Status of
		{atomic,[]} -> {reply,"Incorrect Username or Password-Failed to LogOut",State};
		{atomic,[_Record]} -> CheckLoginStatus = ets:lookup(loginorlogout,UserName),
					case CheckLoginStatus of
						[_record] -> NewState = ets:delete_object(loginorlogout,{loginorlogout,UserName}),{reply,"Successfully Logged Out",NewState};
						[] -> {reply,"User is Logged out, Login before Logout",State}
					end
	end;

handle_call({check_identity,_SenderName,ReceiverName,_Message},_From,State) ->
	Status = mnesia:transaction(fun() -> qlc:e(qlc:q([X || X<-mnesia:table(registerorderegister),X#registerorderegister.username =:= ReceiverName])) end),
	case Status of
		{atomic,[]} -> {reply,"Unknown User",State};
		_ -> LoginStatus = ets:lookup(loginorlogout,ReceiverName),
			case LoginStatus of
				[] -> {reply,"user is offline",State};
				[_Record] -> {reply,"user is online",State}
			end
	end;

handle_call({conversation,SenderName,ReceiverName,Message},_From,State) ->
	CheckLoginStatus = ets:lookup(loginorlogout,ReceiverName),
	{atomic,Get_SocketInfo_of_ReceiverName} = mnesia:transaction(fun() -> qlc:e(qlc:q([X#registerorderegister.socket_details || X<-mnesia:table(registerorderegister), X#registerorderegister.username =:= ReceiverName])) end),
	
	case CheckLoginStatus of
		[] -> {reply,"User is offline",State};
		[_record] -> {reply,{hd(Get_SocketInfo_of_ReceiverName),"Successfully sent the message"},State}
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
        {aborted, {already_exists, _Chat_message}} -> 
            io:format("Table already exists, skipping...~n")
    end.
