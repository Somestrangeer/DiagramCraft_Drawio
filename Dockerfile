FROM alpine:latest

RUN apk add --no-cache \
    g++ \
    make \
    cmake \
    boost-dev \
    boost-json \
    boost-program_options \
    curl-dev \
    nlohmann-json \
    socat  # Добавляем socat для проброса портов

WORKDIR /app
COPY . .

RUN g++ -std=c++20 -o app \
    -I/usr/include/boost \
    -L/usr/lib \
    *.cpp \
    -lboost_system \
    -lboost_json \
    -lcurl

# Запускаем приложение и пробрасываем порт через socat
CMD ["sh", "-c", "/app/app 8081 & socat TCP-LISTEN:8080,fork,reuseaddr TCP:127.0.0.1:8081"]