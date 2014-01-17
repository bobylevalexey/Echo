запуск для echo_poll (для echo_select аналогично): 
gcc echo_poll.c -o echo -std=c99
./echo

выход по ctrl+c

максимум подключаемых клиентов: 10. (можно изменить эту константу перед main)