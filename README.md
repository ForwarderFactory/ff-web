# ff-web

Source code for the [Forwarder Factory](https://forwarderfactory.com) website

## To be done

Don't know how realistic this is, but here's a list of things that I want to see done or work on personally:

- [ ] Implement a way to deduce WAD information without having to ask for it. Things like title, title id, audio, assets, etc.
- [ ] Implement a banner player of some kind. This is hard, I need a genius to help me implement this. (i.e. banner -> video would suffice)
- [ ] 'Send to my Wii' button, which would install the WAD directly to the Wii through the internet.
- [ ] Background is a bit buggy, maybe fix it up a bit?
- [ ] Introduce some kind of REST API that we can both use internally and expose to the public, for potential third-party applications
- [ ] Add translation support (even if only for English at the moment)
- [ ] Perhaps eliminate HTML serving from the C++ backend and push it to the client (already done for the most part)
- [ ] Better test the configuration file, could be buggy?
- [ ] Ensure it scales well enough for production
- [ ] Implement some kind of generic upload form thingy for when users aren't Wii nerds and don't know things deeply.
- [ ] Replace webm conversion with compilable ffmpeg code

## Dependencies

- Boost (beast, asio, system, url) - for networking
- OpenSSL - SSL/TLS and general cryptography
- [bygg](https://jacobnilsson.com/bygg.html) - for constructing HTML structures (automatically downloaded and built)
- yaml-cpp - for configuration files
- SQLite3 - for database (optional, if PostgreSQL is enabled)
- PostgreSQL - for database (optional, if SQLite3 is enabled)
- iconv - for character encoding (probably already installed)
- nlohmann-json - for JSON parsing
- bcrypt (included as a submodule) - more cryptography, hashing passwords
- ffmpeg - validating videos, converting videos
- imagemagick - validating images, converting images
- uglify-js - minifying JavaScript (optional, only needed for production)
- CMake - build system
- C++20 compiler - Would be a pain to program in assembly, wouldn't it?

macOS: `brew install boost openssl yaml-cpp [sqlite3, postgresql] nlohmann-json cmake npm ffmpeg imagemagick`

Debian/Ubuntu: `sudo apt install libboost-all-dev libssl-dev libyaml-cpp-dev libsqlite3-dev libpq-dev nlohmann-json3-dev cmake npm ffmpeg imagemagick libmagick++-dev`

`npm install uglify-js -g` (optional, DO NOT FORGET THE `-g` FLAG!!!)

## Building

```bash
git clone --recursive https://github.com/ForwarderFactory/ff-web; cd ff-web
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local # -DFF_ENABLE_SQLITE3=true -DFF_ENABLE_POSTGRESQL=true
cmake --build .
cmake --install --prefix=/usr/local .
```

If you prefer a debug build (required if you're just testing), replace `Release` with `Debug`
and pass `-De_debug=true` to the cmake command. This will replace the default paths (e.g. `/etc/ff/...` with `./...`).

Static linking is not supported, as much as I'd like to support it.
The reason for this is because of poor library support for static linking, especially with
some dependencies such as Postgres. If you have a good solution however,
please let us know!

## Postgres setup

**Note: I am very new to Postgres, so I don't know if this is ideal or if it will even work for you.**

- `brew install postgresql # install postgres`
- `brew services run postgresql # start postgres`
- `psql postgres # connect to postgres`
- `CREATE DATABASE ff; # create a database`
- `CREATE USER postgres WITH PASSWORD 'password'; # create a user`
- `GRANT ALL PRIVILEGES ON DATABASE ff TO postgres; # grant privileges`
- `\c ff; # connect to the database`
- `... # do stuff`
- `\q # quit`

You can override credentials in your configuration file. Note that SQLite3 may be
enabled by default.

## Note to developers

When I got started with this project, I intended to utilize bygg for constructing HTML structures. As the project developed and
I started utilizing JavaScript more and more, I started to create elements dynamically, as opposed to constructing them in C++.
One part of me is unsatisfied with this (you know, eating your own dog food and all that), but I think this approach works
better for frontend UX.

In the future, perhaps eliminating bygg as a dependency would be a good idea and go all out on the JavaScript side of
things, exclusively using C++ for backend, and using backend as little as possible (push the performance burden to the client
instead of the server).

Also, I am currently suffering from an identity crisis surrounding the use of `snake_case` and `camelCase`. Somebody
please convince me to be consistent. For the moment, the C++ uses only `snake_case` and JavaScript uses both. I'm sorry.

## Contributing

We welcome contributions to this project, of any kind, whether they be bug reports, feature requests, code contributions,
fixes, translations, documentation, trivial or profound. We are happy to have you as a contributor and we don't take
it for granted. We'd like to accommodate contributors' needs and interests as much as possible! Thanks a ton!

For issues related to certain libraries or externally included code, please refer to the respective repository. If you
are not aware of this, feel free to file it here!

## Hosting

Hosting is entirely unrelated to this repository, seeing as this is simply the source code running on the server.
For issues surrounding hosting (i.e. slow, things not working as intended, help I lost my data, etc.) please
email us at `contact@forwarderfactory.com` and someone will hopefully help you out!

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

Copyright (c) 2025 Jacob Nilsson <jacob@jacobnilsson.com> (and future contributors)

Any assets may be licensed under a different license, and are not to be considered a part of this license.
The license exclusively applies to the source code in this repository, and only the source code that is not
explicitly marked as being under a different license. If you are unsure, please file an issue!

This project and the source code and/or binaries may be used by Forwarder Factory for any purpose,
as outlined in the [LICENSE](LICENSE) file
