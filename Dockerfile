# Stage 1: Build
FROM alpine:latest AS build
WORKDIR /usr/src/app
RUN apk add --no-cache gcc libc-dev make
COPY . .
RUN make

# Stage 2: Runtime
FROM frolvlad/alpine-glibc:latest
WORKDIR /usr/src/app

# Create a non-root user
RUN addgroup -S appgroup && adduser -S appuser -G appgroup
USER appuser

# Copy the built binary and the pages directory
COPY --from=build /usr/src/app/coreshot .
COPY --from=build /usr/src/app/pages ./pages

EXPOSE 8080
CMD ["./coreshot"]
