# switcher
Tray icon switcher

Настройка доступа к устройству по ssh.

Для работы понадобится bash интерпретатор. Например, из состава git.
С помощью утилит ssh-* будут установлены ключи шифрования на хост и устройство,
после чего будет возможен доступ по ssh без ввода пароля.
Использование одного и того же ключа для разных устройств -  это нормально.


1. Если уже есть ключи шифрования, то пункт 1 можно пропустить.
Для генерация новой пары открытого и закрытого ключа используется утилита ssh-keygen.
Ключи на хосте хранятся в каталоге пользователя c:\Users\<username>, подкаталог .ssh.
Генерируем ключ сразу в него. Задавать passphrase не нужно.
Для примера приведен вывод сообщений в консоль в случае успешной генерации ключей.

	$ ssh-keygen
	Generating public/private rsa key pair.
	Enter file in which to save the key (/c/Users/alexander/.ssh/id_rsa): /c/Users/alexander/.ssh/yota_key
	Enter passphrase (empty for no passphrase):
	Enter same passphrase again:
	Your identification has been saved in /c/Users/alexander/.ssh/yota_key
	Your public key has been saved in /c/Users/alexander/.ssh/yota_key.pub
	The key fingerprint is:
	SHA256:OadoxR8gOTDL9qM3UxBo+8DoPNRWvlo1MSYthuyuRbI alexander@amax
	The key's randomart image is:
	+---[RSA 3072]----+
	|   .oo..         |
	|   .=+=o=        |
	|   *+==+.o       |
	|  +.O..=oo       |
	| + * oooS.o      |
	|  E o.+= = .     |
	|   +.o* . .      |
	|  . .o o         |
	|                 |
	+----[SHA256]-----+

2. В случае хоста с Windows 11 может потребоваться дополнительная настройка системного ssh
Для этого в каталоге c:\Users\<username>\.ssh необходимо создать файл config и записать в него строки:

	HostKeyAlgorithms +ssh-rsa
	PubkeyAcceptedKeyTypes +ssh-rsa

3. Публичный ключ необходимо записать в список доверенных ключей на устройстве.
Для этого на устройстве должна быть папка /home/<user>/.ssh с правами 700, внутри нее файл authorized_keys с правами 600.
Для автоматизации этого процесса можно использовать утилиту ssh-copy-id.
Запуск с параметроми "-i <путь к ключу> <пользователь>@<имя хоста>". При подключении может быть предложено ввести пароль.
Для примера приведен вывод сообщений в консоль в случае успешной записи ключа.

	$ ssh-copy-id -i /c/Users/alexander/.ssh/yota_key root@yota-hostname
	/usr/bin/ssh-copy-id: INFO: Source of key(s) to be installed: "/c/Users/alexander/.ssh/yota_key.pub"
	/usr/bin/ssh-copy-id: INFO: attempting to log in with the new key(s), to filter out any that are already installed
	/usr/bin/ssh-copy-id: INFO: 1 key(s) remain to be installed -- if you are prompted now it is to install the new keys

	Number of key(s) added: 1

	Now try logging into the machine, with:   "ssh 'root@yota-hostname'"
	and check to make sure that only the key(s) you wanted were added.

4. Для проверки можно запустить команду "ssh -i /c/Users/alexander/.ssh/yota_key root@yota-hostname".
Пароль не должен быть запрошен, сессия ssh должна быть открыта.

5. Добавление устройства в список доверенных хостов.
При подключении к устройству проверяется его отпечаток, если его нет в файле
c:\Users\<username>\.ssh\known_hosts или он отличается, то подключение не будет установлено.
При первом подключении пользователю предлагают добавить хост в список.
Можно заранее добавить отпечаток устройства с помощью утилиты ssh-keyscan.
Команда "ssh-keyscan -H <имя хоста> >> /c/Users/alexander/.ssh/known_hosts".
Чтобы не проверять отпечаток можно при запуске ssh указать параметр "-o StrictHostKeyChecking=no".
