#!/usr/bin/bash

cp ./other/program.sh /tmp/program1.sh
cp ./other/program.sh /tmp/program2.sh
cp ./other/program.sh /tmp/program3.sh
mkdir -p /tmp/program_{in,out}
rm -f /tmp/myinit.log

gcc myinit.c -o myinit

cp other/3procs.conf myinit.conf
./myinit $(realpath ./myinit.conf)

sleep 3
child_c=$(pgrep -P $(pgrep myinit) | wc -l)
[ $child_c -eq 3 ] && echo 'Запущено 3 дочерних процесса: OK'

kill $(pgrep -P $(pgrep myinit) | tail -n +2 | head -n -1)
echo 'Убили 2ой процесс'

sleep 1
child_c=$(pgrep -P $(pgrep myinit) | wc -l)
[ $child_c -eq 3 ] && echo 'Запущено 3 дочерних процесса: OK'

cp other/1procs.conf myinit.conf
kill -SIGHUP $(pgrep myinit)
echo 'Сигнал SIGHUP отправлен'

sleep 1
child_c=$(pgrep -P $(pgrep myinit) | wc -l)
[ $child_c -eq 1 ] && echo 'Запущен 1 дочерний процесс: OK'

sleep 3
kill  $(pgrep myinit)
echo 'Остановили myinit'

echo -e "\nСодержимое файла /tmp/myinit.log ниже\n"
cat /tmp/myinit.log