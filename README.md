
# TCP PROXY-SERVER

Реализация TCP прокси-сервера для СУБД PostgreSQL с использованием Berkeley sockets и режимом мультиплексированого ввода-вывода (на основе poll)

Сервер логирует SQL запросы на основе [клиент-серверного протокола](https://postgrespro.ru/docs/postgresql/16/protocol)  

### Quick start

Сделать сборку проекта. 
В каталоге `build` через терминал запустить:

```bash
$ ./proxy
```

### Building

#### Сборка вручную

```bash
$ cd tcp-proxy-postgres
$ mkdir build
$ cmake ..
$ make
```

### Settings

Настройки сервера по умолчанию:
```
Port: 8080
Ip: 127.0.0.1
```

Для Postgres:

```
Port: 5432
Ip: 127.0.0.1
```

> [!NOTE] 
> Предварительно необходимо отключить SSL 