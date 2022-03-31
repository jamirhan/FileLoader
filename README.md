##### Зависимости:

Boost 1.42.0

C++17

#### Описание:

Предназначен для взаимодействия между сервером и клиентом: клиент через несколько потоков запрашивает данные с сервера и асинхронно записывает в локальный файл. Для сетевого взаимодействия и асинхронности используется библиотека boost::asio. Далее идет подробная семантика публичных методов.

##### Клиент:

Для создания клиента необходимо создать контекст исполнения $io_service$, задать адрес и порт сервера, количество потоков, а также пути к файлам: к файлу на сервере и к локальному файлу, в который будет скопирована информация с сервера.

Необходимо было скачивать файл в многопоточном режиме. Поэтому клиент создает $threads_num$ потоков, и в каждом запускает сущность $FileClient$, которая инкапсулирует однопоточную работу со скачиванием кусочка файла (Эти кусочки распределяются сервером и идут последовательно, подробный алгоритм разделения в следующем разделе). Поэтому у FileClient есть один публичный метод - $FetchFileChunk$. Каждый $FileClient$ делает запросы на отдельном сокете.

##### Сервер:

Для создания сервера предварительно нужно создать контекст исполнения $io_service$. В конструктор также надо передать порт. 

Сервер на каждое подключение асинхронно создает сессию, которая инкапсулирует работу с клиентом.

Все сессии контролируются классом $SessionOperator$. Это необходимо, так как два различных новых подлючения могут происходить из одного клиента по схеме, описанной в предыдущем разделе. Таким образом, $SessionOperator$ для каждого $id$ хранит все сессии, которые имеют этот $id$.

После того, как пришло нужное ($n$) количество сессий (а это определяет сам клиент с помощью протокола), в каждой сессии оператор делит файл на $n$ кусков, после чего запускает в каждой сессии $SendChunk$, который отправляет клиентам их куски файла по 1 МБ. 

##### Протокол взаимодействия:

Сессии и клиент имеют протокол взаимодействия:

1) Клиент подключается и запрашивает свой $id$
2) Последний подключенный клиент отправляет сигнал о том, что больше клиентов не будет.
3) Сессия запрашивает размер имени файла на сервере
4) Сессия запрашивает имя файла на сервере
5) Клиент запрашивает $file_pos$, с которого ему записывать в файл
6) Клиент до тех пор, пока не запишет весь чанк, читает в цикле то, что пришлет сервер, и записывает в свой файл
7) Сессия завершается