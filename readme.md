Клиент - программа, запускаемая из консоли. 
Сервер - демон, корректно завершающийся по сигналам SIGTERM и SIGHUP. Сервер многопоточен (по потоку на клиента).

Клиент передаёт файл по TCP серверу. Сервер принимает файл и сохраняет его.

Сборка:
	make
Использование:
	./serverX -a [IP-адрес на котором слушает сервер] -p [Порт на котором слушает сервер]
	
	./client -a [IP-адрес для подключения к серверу] -p [Порт для подключения к серверу]
	

Если -a или -p не указаны, адрес и порт берутся из переменных среды L2ADDR и L2PORT соответственно. Если переменных окружения не существует и адрес с портом не указаны в опциях, программа выдаёт ошибку.

