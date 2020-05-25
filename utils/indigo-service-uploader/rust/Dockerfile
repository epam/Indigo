FROM debian:jessie

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -qq

RUN apt-get install -y --no-install-recommends \
    build-essential \
    curl \
    ca-certificates \
    libssl-dev \
    git

WORKDIR /opt

ENV RUST_VERSION=rust-1.7.0-x86_64-unknown-linux-gnu

RUN curl -sO http://static.rust-lang.org/dist/$RUST_VERSION.tar.gz && \
tar -xzf $RUST_VERSION.tar.gz && \
./$RUST_VERSION/install.sh --without=rust-docs 


RUN DEBIAN_FRONTEND=noninteractive apt-get autoremove -y && \
  rm -rf \
    $RUST_VERSION \
    $RUST_VERSION.tar.gz \
    /var/lib/apt/lists/* \
    /tmp/* \
    /var/tmp/* \
    mkdir /code

COPY . /code

WORKDIR /code
#RUN cargo build --release

