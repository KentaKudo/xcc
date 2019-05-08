FROM ubuntu:18.04

RUN apt-get update && apt-get -y install sudo

RUN sudo apt-get -y install \
  gcc \
  make \
  git \
  binutils \
  libc6-dev

WORKDIR /xcc
