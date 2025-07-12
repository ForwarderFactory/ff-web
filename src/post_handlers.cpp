#include <ff.hpp>
#include <scrypto.hpp>
#include <limhamn/http/http_utils.hpp>
#include <nlohmann/json.hpp>
#include <endpoint_handlers.hpp>

limhamn::http::server::response ff::handle_try_upload_post_endpoint(const limhamn::http::server::request& req, database& db) {
	limhamn::http::server::response response;
	response.content_type = "application/json";

    std::string _json{};

    struct FilePtr {
        std::string file_path{};
        std::string file_name{};
    };

    std::vector<FilePtr> fh{};

    std::string username{};
    bool auth{false};

    if (username_is_stored(req)) { // is session cookie
        username = req.session.at("username");
        const std::string key = req.session.at("key");

        if (!verify_key(db, username, key)) {
	        nlohmann::json json;
        	json["error"] = "FF_INVALID_CREDENTIALS";
        	json["error_str"] = "Invalid credentials provided.";

        	response.body = json.dump();
        	response.http_status = 401;

        	return response;
        }
        auth = true;
    }

#ifdef FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Attempting to upload a file.\n");
#endif

    const auto file_handles = limhamn::http::utils::parse_multipart_form_file(req.raw_body, settings.temp_directory + "/%f-%h-%r");
    for (const auto& it : file_handles) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "File name: " + it.filename + ", Name: " + it.name + "\n");
#endif
        if (it.name == "json") {
            _json = ff::open_file(it.path);
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got JSON\n");
#endif
        } else {
            logger.write_to_log(limhamn::logger::type::warning, "Got unknown file name: " + it.name + "\n");
            fh.push_back({.file_path = it.path, .file_name = it.filename});
        }
    }

    if (_json.empty()) {
    	nlohmann::json json;

		json["error"] = "FF_INVALID_JSON";
		json["error_str"] = "Invalid JSON provided.";

		response.body = json.dump();
		response.http_status = 400;

		return response;
    }

    nlohmann::json json;
    nlohmann::json db_json;
    try {
        json = nlohmann::json::parse(_json);
    } catch (const std::exception&) {
    	nlohmann::json ret_json;

		 ret_json["error"] = "FF_INVALID_JSON";
		 ret_json["error_str"] = "Invalid JSON provided.";

		response.body =  ret_json.dump();
		response.http_status = 400;

		return response;
    }

    if (!auth) {
        if (json.find("username") == json.end() || !json.at("username").is_string()) {
        	nlohmann::json ret_json;
        	ret_json["error"] = "FF_INVALID_CREDENTIALS";
        	ret_json["error_str"] = "Invalid credentials provided.";
        	response.body = ret_json.dump();
        	response.http_status = 401;
        	return response;
        }
        if (json.find("key") == json.end() || !json.at("key").is_string()) {
        	nlohmann::json ret_json;
        	ret_json["error"] = "FF_INVALID_CREDENTIALS";
        	ret_json["error_str"] = "Invalid credentials provided.";
        	response.body = ret_json.dump();
        	response.http_status = 401;
        	return response;
        }

        username = json.at("username").get<std::string>();
        const std::string key = json.at("key").get<std::string>();

        if (!verify_key(db, username, key)) {
        	nlohmann::json ret_json;
        	ret_json["error"] = "FF_INVALID_CREDENTIALS";
        	ret_json["error_str"] = "Invalid credentials provided.";
        	response.body = ret_json.dump();
        	response.http_status = 401;
        	return response;
        }
    }

	if (fh.size() > 10) {
		nlohmann::json ret_json;
		ret_json["error"] = "FF_TOO_MANY_FILES";
		ret_json["error_str"] = "You have uploaded too many files. The maximum is 10.";

		response.body = ret_json.dump();
		response.http_status = 401;
		return response;
	}

	std::string title{};
    std::string text{};
    std::string post_id = scrypto::generate_random_string(4);
	std::string topic_id{};

    if (json.contains("title") && json.at("title").is_string()) {
        title = json.at("title").get<std::string>();
    }

    if (json.contains("text") && json.at("text").is_string()) {
        text = json.at("text").get<std::string>();
    }

    if (json.contains("post_id") && json.at("post_id").is_string()) {
        post_id = json.at("post_id").get<std::string>();
    }

	if (json.contains("topic_id") && json.at("topic_id").is_string()) {
		topic_id = json.at("topic_id").get<std::string>();
	} else {
		nlohmann::json ret_json;
		ret_json["error"] = "FF_MISSING_TOPIC_ID";
		ret_json["error_str"] = "Missing topic ID.";
		response.body = ret_json.dump();
		response.http_status = 400;
		return response;
	}

    const auto check_if_topic_exists = [&db, &topic_id]() -> bool {
        try {
            nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));
            return !db_json.empty();
        } catch (const std::exception&) {
            return false;
        }
    };

	if (!check_if_topic_exists()) {
		nlohmann::json ret_json;
		ret_json["error"] = "FF_TOPIC_NOT_FOUND";
		ret_json["error_str"] = "Topic not found.";
		response.body = ret_json.dump();
		response.http_status = 404;
		return response;
	}

	const auto check_if_post_exists = [&db, &post_id]() -> bool {
        try {
            nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
            return !db_json.empty();
        } catch (const std::exception&) {
            return false;
        }
    };

    int i = 4;
    while (check_if_post_exists()) {
        post_id = scrypto::generate_random_string(i);
        ++i;
    }

	bool open = true;
	if (json.contains("open") && json.at("open").is_boolean()) {
		open = json.at("open").get<bool>();
	}

    db_json["title"] = limhamn::http::utils::htmlspecialchars(title);
    db_json["text"] = limhamn::http::utils::htmlspecialchars(text);
    db_json["created_by"] = username;
    db_json["created_at"] = scrypto::return_unix_millis();
    db_json["identifier"] = post_id;
    db_json["open"] = open;
	db_json["comments"] = nlohmann::json::array();
    db_json["topic_id"] = topic_id;
    db_json["data"] = nlohmann::json::array();

    for (const auto& it : fh) {
        std::string data_key = ff::upload_file(db, FileConstruct{
            .path = it.file_path,
            .name = it.file_name,
            .username = username,
            .ip_address = req.ip_address,
            .user_agent = req.user_agent,
        });

        if (data_key.empty()) {
        	nlohmann::json ret_json;
        	ret_json["error"] = "FF_FILE_UPLOAD_FAILED";
        	ret_json["error_str"] = "File upload failed.";
        	response.body = ret_json.dump();
        	response.http_status = 500;
        	return response;
        }

        db_json["data"].push_back({
            {"download_key", data_key},
            {"filename", it.file_name}
        });
    }

	// insert to db
	try {
		nlohmann::json topic_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));

		if (topic_json.empty()) {
			nlohmann::json ret;
			ret["error_str"] = "Topic not found";
			ret["error"] = "FF_TOPIC_NOT_FOUND";
			response.http_status = 404;
			response.body = ret.dump();
			return response;
		}
		if ((topic_json.find("open") != topic_json.end() && !topic_json.at("open").get<bool>()) && get_user_type(db, username) != UserType::Administrator) {
			nlohmann::json ret;
			ret["error_str"] = "Topic is closed";
			ret["error"] = "FF_TOPIC_CLOSED";
			response.http_status = 403;
			response.body = ret.dump();
			return response;
		}

		if (topic_json.find("posts") == topic_json.end() || !topic_json.at("posts").is_array()) {
			topic_json["posts"] = nlohmann::json::array();
		}
		topic_json["posts"].push_back(post_id);

		ff::logger.write_to_log(limhamn::logger::type::notice, "Inserting post with ID: " + post_id + " into the database.\n");

		db.exec("INSERT INTO posts (identifier, json) VALUES (?, ?);", post_id, db_json.dump());

		ff::logger.write_to_log(limhamn::logger::type::notice, "Post with ID: " + post_id + " inserted into the database.\n");

		ff::set_json_in_table(db, "topics", "identifier", topic_id, topic_json.dump());
	} catch (const std::exception& e) {
		nlohmann::json ret;
		ret["error_str"] = "Failed to create post: " + std::string(e.what());
		ret["error"] = "FF_DATABASE_ERROR";
		response.http_status = 500;
		response.body = ret.dump();
		return response;
	}

	nlohmann::json ret;
	ret["post_id"] = post_id;
	ret["topic_id"] = topic_id;

	response.http_status = 200;
	response.content_type = "application/json";
	response.body = ret.dump();

	return response;
}

limhamn::http::server::response ff::handle_try_upload_post_comment_endpoint(const limhamn::http::server::request& req, database& db) {
	limhamn::http::server::response response;
	response.content_type = "application/json";

    std::string _json{};

    struct FilePtr {
        std::string file_path{};
        std::string file_name{};
    };

    std::vector<FilePtr> fh{};

    std::string username{};
    bool auth{false};

    if (username_is_stored(req)) { // is session cookie
        username = req.session.at("username");
        const std::string key = req.session.at("key");

        if (!verify_key(db, username, key)) {
	        nlohmann::json json;
        	json["error"] = "FF_INVALID_CREDENTIALS";
        	json["error_str"] = "Invalid credentials provided.";

        	response.body = json.dump();
        	response.http_status = 401;

        	return response;
        }
        auth = true;
    }

#ifdef FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Attempting to upload a file.\n");
#endif

    const auto file_handles = limhamn::http::utils::parse_multipart_form_file(req.raw_body, settings.temp_directory + "/%f-%h-%r");
    for (const auto& it : file_handles) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "File name: " + it.filename + ", Name: " + it.name + "\n");
#endif
        if (it.name == "json") {
            _json = ff::open_file(it.path);
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got JSON\n");
#endif
        } else {
            logger.write_to_log(limhamn::logger::type::warning, "Got unknown file name: " + it.name + "\n");
            fh.push_back({.file_path = it.path, .file_name = it.filename});
        }
    }

    if (_json.empty()) {
    	nlohmann::json json;

		json["error"] = "FF_INVALID_JSON";
		json["error_str"] = "Invalid JSON provided.";

		response.body = json.dump();
		response.http_status = 400;

		return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(_json);
    } catch (const std::exception&) {
    	nlohmann::json ret_json;

		 ret_json["error"] = "FF_INVALID_JSON";
		 ret_json["error_str"] = "Invalid JSON provided.";

		response.body =  ret_json.dump();
		response.http_status = 400;

		return response;
    }

    if (!auth) {
        if (json.find("username") == json.end() || !json.at("username").is_string()) {
        	nlohmann::json ret_json;
        	ret_json["error"] = "FF_INVALID_CREDENTIALS";
        	ret_json["error_str"] = "Invalid credentials provided.";
        	response.body = ret_json.dump();
        	response.http_status = 401;
        	return response;
        }
        if (json.find("key") == json.end() || !json.at("key").is_string()) {
        	nlohmann::json ret_json;
        	ret_json["error"] = "FF_INVALID_CREDENTIALS";
        	ret_json["error_str"] = "Invalid credentials provided.";
        	response.body = ret_json.dump();
        	response.http_status = 401;
        	return response;
        }

        username = json.at("username").get<std::string>();
        const std::string key = json.at("key").get<std::string>();

        if (!verify_key(db, username, key)) {
        	nlohmann::json ret_json;
        	ret_json["error"] = "FF_INVALID_CREDENTIALS";
        	ret_json["error_str"] = "Invalid credentials provided.";
        	response.body = ret_json.dump();
        	response.http_status = 401;
        	return response;
        }
    }

	if (fh.size() > 10) {
		nlohmann::json ret_json;
		ret_json["error"] = "FF_TOO_MANY_FILES";
		ret_json["error_str"] = "You have uploaded too many files. The maximum is 10.";

		response.body = ret_json.dump();
		response.http_status = 401;
		return response;
	}

	std::string post_id{};
	if (json.contains("post_id") && json.at("post_id").is_string()) {
		post_id = json.at("post_id").get<std::string>();
	} else {
		nlohmann::json ret;
		ret["error_str"] = "post_id is required";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	std::string comment{};
	if (json.contains("comment") && json.at("comment").is_string()) {
		comment = json.at("comment").get<std::string>();
	} else {
		nlohmann::json ret;
		ret["error_str"] = "comment is required";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	try {
		nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
		if (db_json.empty()) {
			nlohmann::json ret;
			ret["error_str"] = "Post not found";
			ret["error"] = "FF_POST_NOT_FOUND";
			response.http_status = 404;
			response.body = ret.dump();
			return response;
		}

		if ((db_json.find("open") != db_json.end() && !db_json.at("open").get<bool>()) && get_user_type(db, username) != UserType::Administrator) {
			nlohmann::json ret;
			ret["error_str"] = "Post is closed";
			ret["error"] = "FF_POST_CLOSED";
			response.http_status = 403;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("topic_id") != db_json.end() && db_json.at("topic_id").is_string()) {
			nlohmann::json topic_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", db_json.at("topic_id").get<std::string>()));
			if (topic_json.empty()) {
				nlohmann::json ret;
				ret["error_str"] = "Topic not found";
				ret["error"] = "FF_TOPIC_NOT_FOUND";
				response.http_status = 404;
				response.body = ret.dump();
				return response;
			}

			if (topic_json.find("open") != topic_json.end() && !topic_json.at("open").get<bool>()) {
				nlohmann::json ret;
				ret["error_str"] = "Topic is closed";
				ret["error"] = "FF_TOPIC_CLOSED";
				response.http_status = 403;
				response.body = ret.dump();
				return response;
			}
		}

		if (db_json.find("comments") == db_json.end() || !db_json.at("comments").is_array()) {
            db_json["comments"] = nlohmann::json::array();
        }

		nlohmann::json comment_json;

		comment_json["comment"] = limhamn::http::utils::htmlspecialchars(comment);
		comment_json["created_by"] = username;
		comment_json["created_at"] = scrypto::return_unix_millis();
		comment_json["data"] = nlohmann::json::array();
		for (const auto& it : fh) {
			std::string data_key = ff::upload_file(db, FileConstruct{
				.path = it.file_path,
				.name = it.file_name,
				.username = username,
				.ip_address = req.ip_address,
				.user_agent = req.user_agent,
			});

			if (data_key.empty()) {
				nlohmann::json ret_json;
				ret_json["error"] = "FF_FILE_UPLOAD_FAILED";
				ret_json["error_str"] = "File upload failed.";
				response.body = ret_json.dump();
				response.http_status = 500;
				return response;
			}

			comment_json["data"].push_back({
				{"download_key", data_key},
				{"filename", it.file_name}
			});
		}

		db_json["comments"].push_back(comment_json);

		ff::set_json_in_table(db, "posts", "identifier", post_id, db_json.dump());

		nlohmann::json ret;
		ret["post_id"] = post_id;
		ret["topic_id"] = db_json.at("topic_id").get<std::string>();
		response.http_status = 200;
		response.body = ret.dump();
		return response;
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Post not found";
		ret["error"] = "FF_POST_NOT_FOUND";
		response.http_status = 404;
		response.body = ret.dump();
		return response;
	}
}
