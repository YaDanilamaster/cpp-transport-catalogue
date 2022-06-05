# Транспортный справочник
### Второй проект в рамках учебного курса: транспортный справочник.

*Сейчас проект находится в стадии разработки и на текущий момент реализован следующий функционал.*

Программа загружает данные об автобусах и остановках в формате JSON, строит базу данных автобусных маршрутов, а затем выводит ответы в формате JSON на следующие запросы:

- названия остановок, через которые проходит заданный автобусный маршрут;
- названия автобусных маршрутов, которые проходят через заданную остановку.

В результат запроса включается визуализация запрошенного маршрута (при соответствующем запросе выводится карта всех маршрутов). Результатом работы программы будет SVG-изображение карты, подобное этому:
<img src="https://pictures.s3.yandex.net/resources/illustration_1650925674.svg" alt="cpp-transport-catalogue" width="400" height="400">

Проект реализован в виде консольного приложения, с версией C++17. Использованы только STL.

В будущем планируется построение нужного маршрута из точки А в точку Б с указанием всех возможных вариантов, а также возможные пересадки.
