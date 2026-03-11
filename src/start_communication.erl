-module(start_communication).
-export([start/1]).
-include("chat_application_pb.hrl").
-define(MNESIA_DATABASE,mnesia_database).
start(NewSocket) ->
	case gen_tcp:recv(NewSocket,4) of
		{ok, <<Len:32/big>>} -> {ok, Data} = gen_tcp:recv(NewSocket, Len),
		io:format("RD is ~p ~n",[Data]),
		
					ExtractedData = chat_application_pb:decode_msg(Data,'ChatEnvelope'),
					io:format("RM ~p ~n",[ExtractedData]),
					case ExtractedData of
						{'ChatEnvelope',{auth,AuthExtractedMsg}} ->
							#'Authentication'{request_type = RequestType, username = UserName, password = Password} = AuthExtractedMsg,
							case RequestType of
								"register"    -> user_authentication({register_the_user,UserName,Password}, NewSocket), start(NewSocket);
								"unregister"  -> user_authentication({unregister_the_user,UserName,Password}, NewSocket), start(NewSocket);
								"login"       -> user_authentication({login,UserName,Password}, NewSocket), start(NewSocket);
								"logout"      -> user_authentication({logout,UserName,Password}, NewSocket), start(NewSocket)
							end;
						{'ChatEnvelope',{exchangemsg,ExchangeMessage}} -> 
							#'ExchangeMessage'{message_type = MsgType, conversation = ActualMessage} = ExchangeMessage,
							case MsgType of
								"check_identity" -> user_authentication({list_to_atom(MsgType),ActualMessage},NewSocket), start(NewSocket);
								_ -> ok
							end
					end;	
		_ -> ok
	end.
	
	
%%user_authentication(register,NewSocket,UserName,Password) ->
%%	Reply = analyze_reply(gen_server:call(?MNESIA_DATABASE,{register_the_user,UserName,Password})),
	%%encode_authentication_msg(NewSocket,Reply);
%%user_authentication(unregister,NewSocket,UserName,Password) ->
%%	Reply = analyze_reply(gen_server:call(?MNESIA_DATABASE,{unregister_the_user,UserName,Password})),
%%	encode_authentication_msg(NewSocket,Reply);
%%user_authentication(login,NewSocket,UserName,Password) ->
%%	Reply = analyze_reply(gen_server:call(?MNESIA_DATABASE,{login,UserName,Password})),
%%	encode_authentication_msg(NewSocket,Reply);
user_authentication(Information, NewSocket) ->
	Reply = analyze_reply(gen_server:call(?MNESIA_DATABASE, Information)),
	encode_authentication_msg(NewSocket,Reply).

analyze_reply({ok, Message}) -> Message;
analyze_reply({error, Reason}) -> atom_to_list(Reason);
analyze_reply(Reply) -> Reply.
	
encode_authentication_msg(NewSocket,Reply) ->
	ToBeEncoded = #'AuthenticationReply'{text = Reply},
	EncodedData = chat_application_pb:encode_msg(ToBeEncoded),
	ReplyLen = byte_size(EncodedData),
	gen_tcp:send(NewSocket,<<ReplyLen:32/big,EncodedData/binary>>).
	
