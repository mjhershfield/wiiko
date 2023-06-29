#include "Server.h"

#include <iostream>
#include <network.h>
#include <stdexcept>
#include <string.h>
#include "Game.h"
#include "Request.h"

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
    void join_post_handler(Request& req, Game& game, cJSON* resp_body);
    void state_get_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body);
    void start_post_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body);
	void lobby_get_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body);
	void lobby_post_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body);
    void send_json_response(int32_t client_socket, cJSON* json);
	
	void handle_request(int32_t client_socket, Request& req, Game& game)
	{
		cJSON* resp_body = cJSON_CreateObject();
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
			if (req.body == nullptr)
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
			cJSON_AddBoolToObject(resp_body, "success", false);
			cJSON_AddStringToObject(resp_body, "reason", e.what());
		}

		LWP_MutexUnlock(game.mutex);
		send_json_response(client_socket, resp_body);
		cJSON_Delete(resp_body);
	}

	uint32_t authenticate_user(Request& req, Game& game)
	{
		cJSON* id_node = cJSON_GetObjectItem(req.body, "id");

		if (id_node != nullptr && cJSON_IsNumber(id_node))
		{
			int32_t id = id_node ->valueint;
			if (game.players.is_player(id))
				return id;
			
			throw std::runtime_error("user does not exist");
		}
		else
		{
			throw std::runtime_error("malformed id node");
		}
	}

	void join_post_handler(Request& req, Game& game, cJSON* resp_body)
	{
		cJSON* username_node = cJSON_GetObjectItem(req.body, "username");

		if (username_node == nullptr || !cJSON_IsString(username_node))
			throw std::runtime_error("malformed username node");
		strlwr(username_node->valuestring);
		uint32_t new_player_id = game.add_player(username_node->valuestring);
		cJSON_AddBoolToObject(resp_body, "success", true);
		cJSON_AddNumberToObject(resp_body, "id", new_player_id);
	}

	
	void state_get_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body)
	{
		cJSON_AddBoolToObject(resp_body, "success", true);
		cJSON_AddNumberToObject(resp_body, "state", (double) game.current_state);	
	}

	void start_post_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body)
	{
		if (!game.players.is_admin(player_id))
			throw std::runtime_error("player is not an admin");
		if (game.current_state != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		// TODO: SET GAME NEXT STATE HERE
		game.current_state = game.next_state;
		game.next_state = Game::GameState::Draw;
		cJSON_AddBoolToObject(resp_body, "success", true);
	}

	void lobby_get_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body)
	{
		if (game.current_state != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		cJSON_AddBoolToObject(resp_body, "success", true);
		cJSON_AddBoolToObject(resp_body, "admin", game.players.is_admin(player_id));
		cJSON_AddNumberToObject(resp_body, "pfp", (double) game.players.get_pfp(player_id));
	}

	void lobby_post_handler(Request& req, Game& game, uint32_t player_id, cJSON* resp_body)
	{
		if (game.current_state != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		cJSON* pfp_node = cJSON_GetObjectItem(req.body, "pfp");
		cJSON* quote_node = cJSON_GetObjectItem(req.body, "quote");
		if (cJSON_IsNumber(pfp_node))
			game.players.set_pfp(player_id, (PlayerList::ProfilePicture) pfp_node->valueint);
		else if (cJSON_IsString(quote_node))
			game.players.set_quote(player_id, std::string(quote_node->valuestring));
		else
			throw std::runtime_error("no valid pfp or quote changes");
		cJSON_AddBoolToObject(resp_body, "success", true);
	}

	void send_json_response(int32_t client_socket, cJSON* json)
	{
		net_send(client_socket, http_200, sizeof(http_200) - 1, 0);
		net_send(client_socket, http_json_hdr, sizeof(http_json_hdr) - 1, 0);
		char* json_string = cJSON_PrintUnformatted(json);
		net_send(client_socket, json_string, strlen(json_string), 0);
		free(json_string);
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