#include "Server.h"

#include <iostream>
#include <network.h>
#include <stdexcept>
#include <string.h>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "Game.h"
#include "Request.h"

using namespace rapidjson;

// TODO: a stupid amount of doc comments needed

#define http_200 "HTTP/1.1 200 OK\r\n"

#define http_html_hdr "Content-type: text/html\r\n\r\n"

#define http_json_hdr "Content-type: application/json\r\n\r\n"

namespace Server
{
namespace
{
	void handle_request(int32_t client_socket, Request& req, Game& game);
    uint32_t authenticate_user(Request& req, Game& game);
    void join_post_handler(Request& req, Game& game, Document& resp_body);
    void state_get_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body);
    void start_post_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body);
	void lobby_get_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body);
	void lobby_post_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body);
    void send_json_response(int32_t client_socket, Document& json);
	
	void handle_request(int32_t client_socket, Request& req, Game& game)
	{
		Document resp_body;
		resp_body.SetObject();
		uint32_t player_id;

		#define AUTHENTICATE player_id = authenticate_user(req, game)
		#define CHECK_GET(G) if (req.verb == Get)\
					G(req, game, player_id, resp_body)
		#define CHECK_POST(P) if (req.verb == Post)\
					P(req, game, player_id, resp_body)
		#define INVALID_METHOD throw std::runtime_error("invalid request method")
		#define REGISTER_AUTH_GET(E,G) case E :\
				AUTHENTICATE;\
				CHECK_GET(G);\
				else INVALID_METHOD;\
				break
		#define REGISTER_AUTH_POST(E,P) case E :\
				AUTHENTICATE;\
				CHECK_POST(P);\
				else INVALID_METHOD;\
				break
		#define REGISTER_AUTH_GET_POST(E,G,P) case E :\
				AUTHENTICATE;\
				CHECK_GET(G);\
				else CHECK_POST(P);\
				else INVALID_METHOD;\
				break

		try
		{
			if (req.body.HasParseError())
				throw std::runtime_error("malformed request json");
			while(LWP_MutexLock(game.mutex));
			switch (req.path)
			{
			case Join:
				if (req.verb == Post)
					join_post_handler(req, game, resp_body);
				else
					throw std::runtime_error("invalid method");
				break;

			REGISTER_AUTH_GET(State, state_get_handler);
			REGISTER_AUTH_POST(Start, start_post_handler);
			REGISTER_AUTH_GET_POST(Lobby, lobby_get_handler, lobby_post_handler);
			
			default:
				throw std::runtime_error("invalid path");
				break;
			}
		}
		catch(const std::exception& e)
		{
			resp_body.AddMember("success", Value().SetBool(false), resp_body.GetAllocator());
			resp_body.AddMember("reason", StringRef(e.what()), resp_body.GetAllocator());
		}

		LWP_MutexUnlock(game.mutex);
		send_json_response(client_socket, resp_body);
	}

	uint32_t authenticate_user(Request& req, Game& game)
	{
		Value::ConstMemberIterator id_iter = req.body.FindMember("id");

		if (id_iter != req.body.MemberEnd() && id_iter->value.IsInt())
		{
			int32_t id = id_iter->value.GetInt();
			if (game.players.is_player(id))
				return id;
			
			throw std::runtime_error("user does not exist");
		}
		else
		{
			throw std::runtime_error("malformed id node");
		}
	}

	void join_post_handler(Request& req, Game& game, Document& resp_body)
	{
		Value::ConstMemberIterator username_iter = req.body.FindMember("username");

		if (username_iter == req.body.MemberEnd() || !username_iter->value.IsString())
			throw std::runtime_error("malformed username node");
		uint32_t new_player_id = game.players.add_player(username_iter->value.GetString());
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("id", Value().SetInt(new_player_id), resp_body.GetAllocator());
	}

	
	void state_get_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body)
	{
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("state", Value().SetInt(game.get_current_state()), resp_body.GetAllocator());
	}

	void start_post_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		if (!game.players.is_admin(player_id))
			throw std::runtime_error("player is not an admin");
		// TODO: Prevent game starting unless there are enough players
		// if (game.players.num_players() < 4)
		// 	throw std::runtime_error("not enough players");
		game.go_to_next_state();
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
	}

	void lobby_get_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("admin", Value().SetBool(game.players.is_admin(player_id)), resp_body.GetAllocator());
		resp_body.AddMember("pfp", Value().SetInt(game.players.get_pfp(player_id)), resp_body.GetAllocator());
	}

	void lobby_post_handler(Request& req, Game& game, uint32_t player_id, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		
		Value::ConstMemberIterator pfp_iter = req.body.FindMember("pfp");
		Value::ConstMemberIterator quote_iter = req.body.FindMember("quote");

		if (pfp_iter != req.body.MemberEnd() && pfp_iter->value.IsInt())
			game.players.set_pfp(player_id, (PlayerList::ProfilePicture) pfp_iter->value.GetInt());
		else if (quote_iter != req.body.MemberEnd() && quote_iter->value.IsString())
			game.players.set_quote(player_id, std::string(quote_iter->value.GetString()));
		else
			throw std::runtime_error("no valid pfp or quote changes");
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
	}

	// TODO: Add a draw handler that will provide image data as data url?

	void send_json_response(int32_t client_socket, Document& json)
	{
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		json.Accept(writer);
		net_send(client_socket, http_200, sizeof(http_200) - 1, 0);
		net_send(client_socket, http_json_hdr, sizeof(http_json_hdr) - 1, 0);
		net_send(client_socket, buffer.GetString(), buffer.GetSize(), 0);
	}
}

	void* server_entrypoint(void* game_ptr)
	{
		int32_t sock, client_socket;
		int32_t ret;
		uint32_t clientlen;
		sockaddr_in client;
		sockaddr_in server;
		char buf[1024 * 32];
		size_t buflen;

		Game& game = *((Game*) game_ptr);

		clientlen = sizeof(client);

		sock = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

		if (sock == INVALID_SOCKET)
		{
			std::cout << "Cannot create a socket!" << std::endl;
			return NULL;
		}

		memset(&server, 0, sizeof(server));
		memset(&client, 0, sizeof(client));

		server.sin_family = AF_INET;
		server.sin_port = htons(80);
		server.sin_addr.s_addr = INADDR_ANY;
		ret = net_bind(sock, (sockaddr*)&server, sizeof(server));

		if (ret)
		{
			std::cout << "Error " << ret << " binding socket!" << std::endl;
			return NULL;
		}

		if ((ret = net_listen(sock, 5)))
		{
			std::cout << "Error " << ret << " listening!" << std::endl;
			return NULL;
		}

		while (1)
		{
			client_socket = net_accept(sock, (struct sockaddr *)&client, &clientlen);

			if (client_socket < 0)
			{
				std::cout << "Error connecting socket " << client_socket << std::endl;
				while (1);
			}

			std::cout << "Connecting port " << client.sin_port << " from " <<  inet_ntoa(client.sin_addr) << std::endl;
			memset(buf, 0, 1026);

			buflen = net_recv(client_socket, buf, 1024, 0);
			std::cout << "Received " << buflen << " bytes" << std::endl;

			Request req(buf, buflen);

			std::cout << "Handling request" << std::endl;
			handle_request(client_socket, req, game);

			std::cout << "Closing connection" << std::endl;
			net_close(client_socket);
		}
		
		return NULL;
	}
}