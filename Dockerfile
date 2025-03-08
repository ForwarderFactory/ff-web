FROM ubuntu:latest

LABEL authors="jacob"

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    git \
    libboost-all-dev \
    libssl-dev \
    libyaml-cpp-dev \
    libpq-dev \
    libsqlite3-dev \
    nlohmann-json3-dev \
    libxml2-dev \
    libc6-dev \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/* \
    && npm install -g uglify-js

WORKDIR /app
COPY . /app
RUN rm -rf build; mkdir -p build
WORKDIR /app/build

RUN groupadd -r ff-web && useradd -r -g ff-web ff-web
RUN mkdir -p /etc/ff /var/log/ff /var/db/ff /var/lib/ff

RUN if [ -d "../js" ]; then cp -r ../js /etc/ff/js; fi
RUN if [ -d "../css" ]; then cp -r ../css /etc/ff/css; fi
RUN if [ -d "../audio" ]; then cp -r ../audio /etc/ff/audio; fi
RUN if [ -d "../fonts" ]; then cp -r ../fonts /etc/ff/fonts; fi
RUN if [ -d "../img" ]; then cp -r ../img /etc/ff/img; fi
RUN if [ -d "../ff-web-assets/js" ]; then cp -r ../ff-web-assets/js /etc/ff/js; fi
RUN if [ -d "../ff-web-assets/css" ]; then cp -r ../ff-web-assets/css /etc/ff/css; fi
RUN if [ -d "../ff-web-assets/audio" ]; then cp -r ../ff-web-assets/audio /etc/ff/audio; fi
RUN if [ -d "../ff-web-assets/fonts" ]; then cp -r ../ff-web-assets/fonts /etc/ff/fonts; fi
RUN if [ -d "../ff-web-assets/img" ]; then cp -r ../ff-web-assets/img /etc/ff/img; fi

RUN cmake .. -DCMAKE_BUILD_TYPE=Release
RUN make -j$(nproc)

# make a tarball with the build and libs
RUN mkdir -p bin lib
RUN cp ff-web bin
RUN ldd bin/ff-web | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp '{}' lib/
RUN tar -czf ff-web_linux_$(uname -m).tar bin lib
RUN gzip ff-web_linux_$(uname -m).tar

RUN if [ ! -f "../config.yaml" ]; then ./ff-web -gc > /etc/ff/config.yaml; fi
RUN if [ -f "../config.yaml" ]; then cp ../config.yaml /etc/ff/config.yaml; fi

RUN chown -R ff-web:ff-web /app/build /etc/ff /var/log/ff /var/db/ff /var/lib/ff
RUN chmod -R 755 /app/build /etc/ff /var/log/ff /var/db/ff /var/lib/ff

USER ff-web
EXPOSE 8080
ENTRYPOINT ["./ff-web"]