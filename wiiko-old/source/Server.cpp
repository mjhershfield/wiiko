#include "Server.h"

#include <stdio.h>
#include <unistd.h>
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

#define http_404 "HTTP/1.1 404 Not Found\r\n"

#define http_json_hdr "Content-type: application/json\r\n"

#define content_len_hdr "Content-Length: %u\r\n"

#define http_html_gzip_hdr "Content-type: text/html\r\nContent-Encoding: gzip\r\n"

#define http_jpeg_hdr "Content-type: image/jpeg\r\n"

#define http_cors_hdr "Access-Control-Allow-Origin: *\r\n\r\n"

namespace Server
{
namespace
{
	void handle_request(int32_t client_socket, Request& req, Game& game);
	void root_get_handler(int32_t client_socket);
    void join_post_handler(Request& req, Game& game, Document& resp_body);
    void state_get_handler(Request& req, Game& game, Document& resp_body);
    void start_post_handler(Request& req, Game& game, Document& resp_body);
	void lobby_get_handler(Request& req, Game& game, Document& resp_body);
	void lobby_post_handler(Request& req, Game& game, Document& resp_body);
	void draw_get_handler(Request& req, Game& game, Document& resp_body);
	void draw_post_handler(Request& req, Game& game, Document& resp_body);
	void write_get_handler(Request& req, Game& game, Document& resp_body);
	void write_post_handler(Request& req, Game& game, Document& resp_body);
	void image_get_handler(int32_t client_socket, Request& req);
    void send_json_response(int32_t client_socket, Document& json);
	void send_404(int32_t client_socket);
	
	void handle_request(int32_t client_socket, Request& req, Game& game)
	{
		Document resp_body;
		resp_body.SetObject();

		#define AUTHENTICATE if (!game.players.is_player(req.user_id)) \
					throw std::runtime_error("user does not exist");
		#define CHECK_GET(G) if (req.verb == Get)\
					G(req, game, resp_body)
		#define CHECK_POST(P) if (req.verb == Post)\
					P(req, game, resp_body)
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
			if (req.body_type == Json && req.json_body.HasParseError())
				throw std::runtime_error("malformed request json");
			std::cout << "Waiting for mutex lock" << std::endl;
			while(LWP_MutexLock(game.mutex));
			std::cout << "Got mutex lock" << std::endl;
			switch (req.path)
			{
			case Root:
				root_get_handler(client_socket);
				break;
			case Join:
				if (req.verb == Post)
					join_post_handler(req, game, resp_body);
				else
					throw std::runtime_error("invalid method");
				break;

			case Image:
				if (req.verb == Get)
					image_get_handler(client_socket, req);
				else
					throw std::runtime_error("invalid method");
				break;

			// TODO: ADD TIMEOUT TO WAIT RESPONSE?
			// TODO: how will we serve jpegs? It's gonna be really slow lmao
			REGISTER_AUTH_GET(State, state_get_handler);
			REGISTER_AUTH_POST(Start, start_post_handler);
			REGISTER_AUTH_GET_POST(Lobby, lobby_get_handler, lobby_post_handler);
			REGISTER_AUTH_GET_POST(Draw, draw_get_handler, draw_post_handler);
			REGISTER_AUTH_GET_POST(Write, write_get_handler, write_post_handler);
			
			default:
				send_404(client_socket);
				req.path = UnsupportedPath;
				break;
			}
		}
		catch(const std::exception& e)
		{
			resp_body.AddMember("success", Value().SetBool(false), resp_body.GetAllocator());
			resp_body.AddMember("reason", StringRef(e.what()), resp_body.GetAllocator());
		}

		std::cout << "Unlocking Mutex" << std::endl;
		LWP_MutexUnlock(game.mutex);
		// Don't send JSON response for index.html, 404
		std::cout << "Sending JSON response (if needed)" << std::endl;
		if (req.path != Root && req.path != UnsupportedPath && req.path != Image)
			send_json_response(client_socket, resp_body);
	}

	void root_get_handler(int32_t client_socket)
	{
		char buf[1024];

		FILE* index_html_file = fopen("sd://apps/wiiko/index.html.gz", "rb");
		if (index_html_file == NULL)
		{
			std::cout << "could not find sd://apps/wiiko/index.html.gz" << std::endl;
			send_404(client_socket);
			return;
		}

		std::cout << "Serving Web App" << std::endl;
		net_send(client_socket, http_200, sizeof(http_200) - 1, 0);
		char content_len_with_size[sizeof(content_len_hdr) + 10];
		fseek(index_html_file, 0, SEEK_END);
		size_t index_html_len = ftell(index_html_file);
		rewind(index_html_file);
		std::cout << "Length of index.html: " << index_html_len << std::endl;
		size_t content_len_len = sprintf(content_len_with_size, content_len_hdr, index_html_len);
		net_send(client_socket, content_len_with_size, content_len_len, 0);
		net_send(client_socket, http_html_gzip_hdr, sizeof(http_html_gzip_hdr) - 1, 0);
		net_send(client_socket, http_cors_hdr, sizeof(http_cors_hdr) - 1, 0);

		size_t amount_transferred = 0, amount_to_transfer = 0, index_send_result;
		while (amount_transferred < index_html_len)
		{
			// amount_to_transfer = std::min(1024U, index_html_len - amount_transferred);
			// index_send_result = net_send(client_socket, index_html + amount_transferred, amount_to_transfer, 0);
			// if (index_send_result < 0)
			// {
			// 	std::cout << "Failed to send index" << std::endl;
			// }
			// amount_transferred += index_send_result;
			amount_to_transfer = fread(buf, sizeof(char), sizeof(buf), index_html_file);
			// std::cout << "read " << amount_to_transfer << "bytes from index.html.br";
			index_send_result = net_send(client_socket, buf, amount_to_transfer, 0);
			// std::cout << "sending from " << amount_transferred << " to " << amount_transferred + amount_to_transfer << std::endl;
			if (index_send_result >= 0)
			{
				amount_transferred += index_send_result;
			}
			else
			{
				std::cout << "ERROR WHILE SENDING INDEX.HTML: " << errno << std::endl;
				break;
			}
		}

		// fclose(index_html_file);
	}

	void join_post_handler(Request& req, Game& game, Document& resp_body)
	{
		if (req.body_type != Json)
			throw std::runtime_error("request body is not JSON");

		Value::ConstMemberIterator username_iter = req.json_body.FindMember("username");

		if (username_iter == req.json_body.MemberEnd() || !username_iter->value.IsString())
			throw std::runtime_error("malformed username node");
		uint32_t new_player_id = game.players.add_player(username_iter->value.GetString());
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("id", Value().SetInt(new_player_id), resp_body.GetAllocator());
	}

	
	void state_get_handler(Request& req, Game& game, Document& resp_body)
	{
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("state", Value().SetInt(game.get_current_state()), resp_body.GetAllocator());
	}

	void start_post_handler(Request& req, Game& game, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		if (!game.players.is_admin(req.user_id))
			throw std::runtime_error("player is not an admin");
		// TODO: Prevent game starting unless there are enough players
		// if (game.players.num_players() < 4)
		// 	throw std::runtime_error("not enough players");
		game.go_to_next_state();
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
	}

	void lobby_get_handler(Request& req, Game& game, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("admin", Value().SetBool(game.players.is_admin(req.user_id)), resp_body.GetAllocator());
		resp_body.AddMember("pfp", Value().SetInt(game.players.get_pfp(req.user_id)), resp_body.GetAllocator());
	}

	void lobby_post_handler(Request& req, Game& game, Document& resp_body)
	{
		if (req.body_type != Json)
			throw std::runtime_error("request body is not JSON");

		if (game.get_current_state() != Game::GameState::Lobby)
			throw std::runtime_error("game has already started");
		
		Value::ConstMemberIterator pfp_iter = req.json_body.FindMember("pfp");
		Value::ConstMemberIterator quote_iter = req.json_body.FindMember("quote");

		if (pfp_iter != req.json_body.MemberEnd() && pfp_iter->value.IsInt())
			game.players.set_pfp(req.user_id, (PlayerList::ProfilePicture) pfp_iter->value.GetInt());
		else if (quote_iter != req.json_body.MemberEnd() && quote_iter->value.IsString())
			game.players.set_quote(req.user_id, std::string(quote_iter->value.GetString()));
		else
			throw std::runtime_error("no valid pfp or quote changes");
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
	}

	void draw_get_handler(Request& req, Game& game, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Draw)
			throw std::runtime_error("not currently drawing");
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("timeout", Value().SetUint(game.check_next_state_timeout()), resp_body.GetAllocator());
		resp_body.AddMember("submitted", Value().SetBool(game.player_num_submissions(req.user_id)), resp_body.GetAllocator());
	}

	void draw_post_handler(Request& req, Game& game, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Draw)
			throw std::runtime_error("not currently drawing");

		if (req.body_type != Jpeg)
			throw std::runtime_error("request body is not JPEG");

		if (game.player_num_submissions(req.user_id))
			throw std::runtime_error("player already submitted for this round");

		game.add_image_from_blob(req.user_id, req.raw_body_ptr, req.raw_body_len);

		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
	}

	// TODO: Implement write
	void write_get_handler(Request& req, Game& game, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Write)
			throw std::runtime_error("not currently writing");
		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		resp_body.AddMember("timeout", Value().SetUint(game.check_next_state_timeout()), resp_body.GetAllocator());
	}

	void write_post_handler(Request& req, Game& game, Document& resp_body)
	{
		if (game.get_current_state() != Game::GameState::Write)
			throw std::runtime_error("not currently writing");

		if (req.body_type != Json)
			throw std::runtime_error("request body is not JSON");
		
		Value::ConstMemberIterator slogan_iter = req.json_body.FindMember("slogan");

		if (slogan_iter != req.json_body.MemberEnd() && slogan_iter->value.IsString())
			game.add_slogan(req.user_id, std::string(slogan_iter->value.GetString()));
		else
			throw std::runtime_error("malformed slogan node");

		resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
	}

	// TODO: Implement shirt
	void image_get_handler(int32_t client_socket, Request& req)
	{
		// if (game.get_current_state() != Game::GameState::Shirt)
		// 	throw std::runtime_error("not currently designing shirts");
		// resp_body.AddMember("success", Value().SetBool(true), resp_body.GetAllocator());
		// resp_body.AddMember("timeout", Value().SetUint(game.check_next_state_timeout()), resp_body.GetAllocator());
		char buf[50];
		char content_len_with_size[sizeof(content_len_hdr) + 10];

		//TODO: Validate that path only includes [hex digits].jpg
		//TODO: crashes if tmp/ doesn't exist?

		sprintf(buf, "sd://apps/wiiko/tmp/%s", req.static_file_path);

		FILE* image_file = fopen(buf, "rb");
		if (image_file == NULL)
		{
			std::cout << "could not find " << buf << std::endl;
			send_404(client_socket);
			return;
		}

		std::cout << "Serving Image" << std::endl;
		net_send(client_socket, http_200, sizeof(http_200) - 1, 0);
		fseek(image_file, 0, SEEK_END);
		size_t image_len = ftell(image_file);
		rewind(image_file);
		std::cout << "Length of image: " << image_len << std::endl;
		size_t content_len_len = sprintf(content_len_with_size, content_len_hdr, image_len);
		net_send(client_socket, content_len_with_size, content_len_len, 0);
		net_send(client_socket, http_jpeg_hdr, sizeof(http_jpeg_hdr) - 1, 0);
		net_send(client_socket, http_cors_hdr, sizeof(http_cors_hdr) - 1, 0);

		size_t amount_transferred = 0, amount_to_transfer = 0, index_send_result;
		while (amount_transferred < image_len)
		{
			amount_to_transfer = fread(buf, sizeof(char), sizeof(buf), image_file);
			// std::cout << "read " << amount_to_transfer << "bytes from index.html.br";
			index_send_result = net_send(client_socket, buf, amount_to_transfer, 0);
			// std::cout << "sending from " << amount_transferred << " to " << amount_transferred + amount_to_transfer << std::endl;
			if (index_send_result >= 0)
			{
				amount_transferred += index_send_result;
			}
			else
			{
				std::cout << "ERROR WHILE SENDING IMAGE: " << errno << std::endl;
				break;
			}
		}

		fclose(image_file);
	}

	// TODO: Implement battle

	void send_json_response(int32_t client_socket, Document& json)
	{
		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		json.Accept(writer);
		net_send(client_socket, http_200, sizeof(http_200) - 1, 0);
		net_send(client_socket, http_json_hdr, sizeof(http_json_hdr) - 1, 0);
		net_send(client_socket, http_cors_hdr, sizeof(http_cors_hdr) - 1, 0);
		net_send(client_socket, buffer.GetString(), buffer.GetSize(), 0);
	}

	void send_404(int32_t client_socket)
	{
		net_send(client_socket, http_404, sizeof(http_404) - 1, 0);
		net_send(client_socket, http_cors_hdr, sizeof(http_cors_hdr) - 1, 0);
	}
}

	void* server_entrypoint(void* game_ptr)
	{
		// int32_t sock, client_socket;
		// int32_t ret;
		// uint32_t clientlen;
		// sockaddr_in client;
		// sockaddr_in server;
		// char buf[1024 * 32 + 1];
		// size_t buflen;

		#define MAX_CLIENTS 16

		// Server code mostly ripped off of https://saltyfish123.github.io/Socket_IO_Multiplexing/

		int listenfd, connfd, i, maxi, nready, timeout = -1, ret, on = 1;
		struct sockaddr_in servaddr, cliaddr;
		socklen_t clilen;
		pollsd clients[MAX_CLIENTS];
		char buf[32 * 1024 + 1];
		size_t n;

		Game& game = *((Game*) game_ptr);

		listenfd = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

		if (listenfd == INVALID_SOCKET)
		{
			std::cout << "Cannot create a socket for listening!" << std::endl;
			return NULL;
		}

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(8080);
		servaddr.sin_addr.s_addr = INADDR_ANY;
		ret = net_bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr));

		if (ret)
		{
			std::cout << "Error " << ret << " binding socket!" << std::endl;
			return NULL;
		}

		if ((ret = net_listen(listenfd, MAX_CLIENTS)))
		{
			std::cout << "Error " << ret << " listening!" << std::endl;
			return NULL;
		}

		clients[0].socket = listenfd;
		clients[0].events = POLLIN;
		for (i = 1; i < MAX_CLIENTS; i++)
		{
			clients[i].socket = -1;
		}
		maxi = 0;

		while (1)
		{
			std::cout << "Polling for new connections" << std::endl;
			nready = net_poll(clients, maxi + 1, timeout);
			std::cout << "Poll completed" << std::endl;

			// If the listening socket has an incoming connection, add it to the clients pollsd[]
			if (clients[0].revents & POLLIN)
			{
				clilen = sizeof(cliaddr);
				connfd = net_accept(listenfd, (sockaddr*) &cliaddr, &clilen);
				// TODO: should we check if connfd < 0 like IBM does?
				for (i = 1; i < MAX_CLIENTS; i++)
				{
					if (clients[i].socket == -1)
					{
						clients[i].socket = connfd;
						clients[i].events = POLLIN;
						maxi = (i > maxi) ? i : maxi;
						break;
					}
				}

				// If poll says that no sockets are ready to be read from, poll again.
				if (--nready <= 0)
				{
					continue;
				}
			}

			for (i = 1; i <= maxi; i++)
			{
				if (clients[i].socket == -1)
					continue;

				if (clients[i].revents & (POLLIN | POLLERR))
				{
					// Read socket and check for errors
					if ((n = net_read(clients[i].socket, &buf, sizeof(buf)-1)) < 0)
					{
						if (errno == ECONNRESET)
						{
							net_close(clients[i].socket);
							printf("The connection %d aborted\n", clients[i].socket);
                        	clients[i].socket = -1;
						}
						else if (errno == EWOULDBLOCK)
						{
							printf("Read would block\n");
						}
						else
						{
							printf("Read error and errno is %d\n", errno);
						}
					}
					// If we have reached EOF
					else if (n == 0)
					{
						printf("Reached EOF, closing socket\n");
						net_close(clients[i].socket);
						clients[i].socket = -1;
					}
					else 
					{
						buf[n] = 0;
						printf("Handling request from fd %d\n", clients[i].socket);
						Request req(buf, n);

						handle_request(clients[i].socket, req, game);
						net_close(clients[i].socket);
						clients[i].socket = -1;
                	}

					if (--nready <= 0)
					{
						break;
					}
				}
			}
		}
		
		return NULL;
	}
}