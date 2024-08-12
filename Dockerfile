FROM debian:latest
WORKDIR /usr/src/app
RUN apt-get update && apt-get install -y gcc libc-dev make
COPY . .
RUN make
EXPOSE 8080
CMD ["./coreshot"]