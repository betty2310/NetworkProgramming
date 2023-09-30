FROM ubuntu:22.04

WORKDIR /network_programming

RUN apt-get update && apt-get install -y build-essential

COPY . /network_programming

VOLUME /network_programming

CMD ["bash"]
