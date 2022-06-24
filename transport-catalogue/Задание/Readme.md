### Задание

Задание посвящено переводу приложения на ввод-вывод в формате JSON.

Изменения формата ввода: Новая секция — routing_settings

Во входной JSON добавляется ключ routing_settings, значение которого — словарь с двумя ключами:
- bus_wait_time — время ожидания автобуса на остановке, в минутах. Считайте, что когда бы человек ни пришёл на остановку и какой бы ни была эта остановка, он будет ждать любой автобус в точности указанное количество минут. Значение — целое число от 1 до 1000.
- bus_velocity — скорость автобуса, в км/ч. Считайте, что скорость любого автобуса постоянна и в точности равна указанному числу. Время стоянки на остановках не учитывается, время разгона и торможения тоже. Значение — вещественное число от 1 до 1000.

Пример
```
"routing_settings": {
      "bus_wait_time": 6,
      "bus_velocity": 40
} 
```
Данная конфигурация задаёт время ожидания, равным 6 минутам, и скорость автобусов, равной 40 километрам в час.

Новый тип запросов к базе — Route

В список stat_requests добавляются элементы с "type": "Route" — это запросы на построение маршрута между двумя остановками. Помимо стандартных свойств id и type, они содержат ещё два:
- from — остановка, где нужно начать маршрут.
- to — остановка, где нужно закончить маршрут.
Оба значения — названия существующих в базе остановок. Однако они, возможно, не принадлежат ни одному автобусному маршруту.

Пример
```
{
      "type": "Route",
      "from": "Biryulyovo Zapadnoye",
      "to": "Universam",
      "id": 4
} 
```
Данный запрос означает построение маршрута от остановки “Biryulyovo Zapadnoye” до остановки “Universam”.
На маршруте человек может использовать несколько автобусов. Один автобус даже можно использовать несколько раз, если на некоторых участках он делает большой крюк и проще срезать на другом автобусе.

Маршрут должен быть наиболее оптимален по времени. Если маршрутов с минимально возможным суммарным временем несколько, допускается вывести любой из них: тестирующая система проверяет лишь совпадение времени маршрута с оптимальным и корректность самого маршрута.

При прохождении маршрута время расходуется на два типа активностей:
- Ожидание автобуса. Всегда длится bus_wait_time минут.
- Поездка на автобусе. Всегда длится ровно такое количество времени, которое требуется для преодоления данного расстояния со скоростью bus_velocity. 

Расстояние между остановками вычисляется не по геолакации, а по дорогам, то есть с использованием road_distances.
Ходить пешком, выпрыгивать из автобуса между остановками и использовать другие виды транспорта запрещается. 

На конечных остановках все автобусы высаживают пассажиров и уезжают в парк. Даже если человек едет на кольцевом — "is_roundtrip": true — маршруте и хочет проехать мимо конечной, он будет вынужден выйти и подождать тот же самый автобус ровно bus_wait_time минут. Этот и другие случаи разобраны в примерах.

Ответ на запрос Route устроен следующим образом:
```
{
    "request_id": <id запроса>,
    "total_time": <суммарное время>,
    "items": [
        <элементы маршрута>
    ]
}
```
- total_time — суммарное время в минутах, которое требуется для прохождения маршрута, выведенное в виде вещественного числа. Обратите внимание, что расстояние от остановки A до остановки B может быть не равно расстоянию от B до A!
- items — список элементов маршрута, каждый из которых описывает непрерывную активность пассажира, требующую временных затрат.

А именно элементы маршрута бывают двух типов.
- Wait — подождать нужное количество минут (в нашем случае всегда bus_wait_time) на указанной остановке:
```
{
    "type": "Wait",
    "stop_name": "Biryulyovo",
    "time": 6
} 
```
- Bus — проехать span_count остановок (перегонов между остановками) на автобусе bus, потратив указанное количество минут:
```
{
    "type": "Bus",
    "bus": "297",
    "span_count": 2,
    "time": 5.235
} 
```
Если маршрута между указанными остановками нет, выведите результат в следующем формате:

```
{
    "request_id": <id запроса>,
    "error_message": "not found"
} 
```
### Реализация

Задача поиска оптимального маршрута данного вида сводится к задаче поиска кратчайшего пути во взвешенном ориентированном графе.

Вам предоставляются две небольшие библиотеки:

graph.h — класс, реализующий взвешенный ориентированный граф,

router.h — класс, реализующий поиск кратчайшего пути во взвешенном ориентированном графе.

Эти библиотеки не нуждаются в модификации. С другой стороны, вы можете не использовать их вовсе или модифицировать произвольным образом.
Как часто бывает в реальной жизни разработчиков, обе эти библиотеки даны без документации, но с самодокументируемым кодом: интерфейсы представленных классов легко читаются по заголовкам публичных методов.

О классах дополнительно известно следующее:

- Вершины и рёбра графа нумеруются автоинкрементно беззнаковыми целыми числами, хранящимися в типах VertexId и EdgeId:
  - вершины нумеруются от нуля до количества вершин минус один в соответствии с пользовательской логикой.
  - Номер очередного ребра выдаётся методом AddEdge; он равен нулю для первого вызова метода и при каждом следующем вызове увеличивается на единицу.
- Память, нужная для хранения графа, линейна относительно суммы количеств вершин и рёбер.
- Конструктор и деструктор графа имеют линейную сложность, а остальные методы константны или амортизированно константны.
- Маршрутизатор — класс Router — требует квадратичного относительно количества вершин объёма памяти, не считая памяти, требуемой для хранения кэша маршрутов.
- Конструктор маршрутизатора имеет сложность O(V^3 + E), где V — количество вершин графа, E — количество рёбер. Маршрутизатор не работает с графами, имеющими рёбра отрицательного веса.
- Построение маршрута на готовом маршрутизаторе линейно относительно количества рёбер в маршруте. Таким образом, основная нагрузка построения оптимальных путей ложится на конструктор.

### Нужно:

- самостоятельно разобраться с понятиями графов и путями в них;

- придумать, как по транспортному справочнику построить граф, в котором путь наименьшего веса соответствует оптимальному маршруту на автобусах. Но сначала надо определиться, что в этом графе будет вершинами, а что — рёбрами;

- написать код построения графа и описания маршрута по пути, полученному от маршрутизатора.

Вершины графа в той или иной степени должны соответствовать остановкам. Поразмышляйте над двумя вариантами, каждый из которых может привести к правильному решению:

- Если ограничиться одной вершиной на остановку, нужно заложить время ожидания автобуса в веса ребёр, соединяющие остановки.

- Можно использовать две вершины для каждой из остановок. Первая соответствует состоянию «начал ждать автобус на остановке S», вторая — «садится в автобус на остановке S». Время ожидания автобуса учитывается благодаря ребру веса bus_wait_time из первой вершины во вторую.

Подумайте, как именно должны быть организованы рёбра графа в каждом из случаев.

### Ограничения
Количество запросов на создание автобусов и остановок — не более 100.

Количество запросов к уже созданной базе — не более 2000, из них запросов на отрисовку — не более двух.

Не более 100 остановок в маршруте.

Ограничение по времени — 3 секунды на тест, по памяти — 512 МБ.

В реальном маршрутизаторе допустимое время на предварительную подготовку данных вполне могло бы занимать минуты, что позволило бы увеличить ограничение на количество объектов.

#### Пример 1
Иллюстрации к примеру прилагаются

Ввод
```
  {
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "297",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryulyovo Zapadnoye"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "635",
              "stops": [
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Prazhskaya"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.574371,
              "longitude": 37.6517,
              "name": "Biryulyovo Zapadnoye",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 2600
              },
              "type": "Stop"
          },
          {
              "latitude": 55.587655,
              "longitude": 37.645687,
              "name": "Universam",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 1380,
                  "Biryulyovo Zapadnoye": 2500,
                  "Prazhskaya": 4650
              },
              "type": "Stop"
          },
          {
              "latitude": 55.592028,
              "longitude": 37.653656,
              "name": "Biryulyovo Tovarnaya",
              "road_distances": {
                  "Universam": 890
              },
              "type": "Stop"
          },
          {
              "latitude": 55.611717,
              "longitude": 37.603938,
              "name": "Prazhskaya",
              "road_distances": {},
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 40,
          "bus_wait_time": 6
      },
      "stat_requests": [
          {
              "id": 1,
              "name": "297",
              "type": "Bus"
          },
          {
              "id": 2,
              "name": "635",
              "type": "Bus"
          },
          {
              "id": 3,
              "name": "Universam",
              "type": "Stop"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 4,
              "to": "Universam",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 5,
              "to": "Prazhskaya",
              "type": "Route"
          }
      ]
  }
```   
Вывод
```
  [
      {
          "curvature": 1.42963,
          "request_id": 1,
          "route_length": 5990,
          "stop_count": 4,
          "unique_stop_count": 3
      },
      {
          "curvature": 1.30156,
          "request_id": 2,
          "route_length": 11570,
          "stop_count": 5,
          "unique_stop_count": 3
      },
      {
          "buses": [
              "297",
              "635"
          ],
          "request_id": 3
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 6,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 2,
                  "time": 5.235,
                  "type": "Bus"
              }
          ],
          "request_id": 4,
          "total_time": 11.235
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 6,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 2,
                  "time": 5.235,
                  "type": "Bus"
              },
              {
                  "stop_name": "Universam",
                  "time": 6,
                  "type": "Wait"
              },
              {
                  "bus": "635",
                  "span_count": 1,
                  "time": 6.975,
                  "type": "Bus"
              }
          ],
          "request_id": 5,
          "total_time": 24.21
      }
  ]
```   
#### Пример 2

Иллюстрации к примеру прилагаются

Ввод
```
{
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "297",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryusinka",
                  "Apteka",
                  "Biryulyovo Zapadnoye"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "635",
              "stops": [
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryusinka",
                  "TETs 26",
                  "Pokrovskaya",
                  "Prazhskaya"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "828",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "TETs 26",
                  "Biryusinka",
                  "Universam",
                  "Pokrovskaya",
                  "Rossoshanskaya ulitsa"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.574371,
              "longitude": 37.6517,
              "name": "Biryulyovo Zapadnoye",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 2600,
                  "TETs 26": 1100
              },
              "type": "Stop"
          },
          {
              "latitude": 55.587655,
              "longitude": 37.645687,
              "name": "Universam",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 1380,
                  "Biryusinka": 760,
                  "Pokrovskaya": 2460
              },
              "type": "Stop"
          },
          {
              "latitude": 55.592028,
              "longitude": 37.653656,
              "name": "Biryulyovo Tovarnaya",
              "road_distances": {
                  "Universam": 890
              },
              "type": "Stop"
          },
          {
              "latitude": 55.581065,
              "longitude": 37.64839,
              "name": "Biryusinka",
              "road_distances": {
                  "Apteka": 210,
                  "TETs 26": 400
              },
              "type": "Stop"
          },
          {
              "latitude": 55.580023,
              "longitude": 37.652296,
              "name": "Apteka",
              "road_distances": {
                  "Biryulyovo Zapadnoye": 1420
              },
              "type": "Stop"
          },
          {
              "latitude": 55.580685,
              "longitude": 37.642258,
              "name": "TETs 26",
              "road_distances": {
                  "Pokrovskaya": 2850
              },
              "type": "Stop"
          },
          {
              "latitude": 55.603601,
              "longitude": 37.635517,
              "name": "Pokrovskaya",
              "road_distances": {
                  "Rossoshanskaya ulitsa": 3140
              },
              "type": "Stop"
          },
          {
              "latitude": 55.595579,
              "longitude": 37.605757,
              "name": "Rossoshanskaya ulitsa",
              "road_distances": {
                  "Pokrovskaya": 3210
              },
              "type": "Stop"
          },
          {
              "latitude": 55.611717,
              "longitude": 37.603938,
              "name": "Prazhskaya",
              "road_distances": {
                  "Pokrovskaya": 2260
              },
              "type": "Stop"
          },
          {
              "is_roundtrip": false,
              "name": "750",
              "stops": [
                  "Tolstopaltsevo",
                  "Rasskazovka"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.611087,
              "longitude": 37.20829,
              "name": "Tolstopaltsevo",
              "road_distances": {
                  "Rasskazovka": 13800
              },
              "type": "Stop"
          },
          {
              "latitude": 55.632761,
              "longitude": 37.333324,
              "name": "Rasskazovka",
              "road_distances": {},
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 30,
          "bus_wait_time": 2
      },
      "stat_requests": [
          {
              "id": 1,
              "name": "297",
              "type": "Bus"
          },
          {
              "id": 2,
              "name": "635",
              "type": "Bus"
          },
          {
              "id": 3,
              "name": "828",
              "type": "Bus"
          },
          {
              "id": 4,
              "name": "Universam",
              "type": "Stop"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 5,
              "to": "Apteka",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 6,
              "to": "Pokrovskaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 7,
              "to": "Pokrovskaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 8,
              "to": "Biryulyovo Zapadnoye",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 9,
              "to": "Prazhskaya",
              "type": "Route"
          },
          {
              "from": "Apteka",
              "id": 10,
              "to": "Biryulyovo Tovarnaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 11,
              "to": "Tolstopaltsevo",
              "type": "Route"
          }
      ]
  }
```   
Вывод
```
  [
      {
          "curvature": 1.36159,
          "request_id": 1,
          "route_length": 5880,
          "stop_count": 6,
          "unique_stop_count": 5
      },
      {
          "curvature": 1.12195,
          "request_id": 2,
          "route_length": 14810,
          "stop_count": 11,
          "unique_stop_count": 6
      },
      {
          "curvature": 1.31245,
          "request_id": 3,
          "route_length": 15790,
          "stop_count": 11,
          "unique_stop_count": 6
      },
      {
          "buses": [
              "297",
              "635",
              "828"
          ],
          "request_id": 4
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "828",
                  "span_count": 2,
                  "time": 3,
                  "type": "Bus"
              },
              {
                  "stop_name": "Biryusinka",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 0.42,
                  "type": "Bus"
              }
          ],
          "request_id": 5,
          "total_time": 7.42
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "828",
                  "span_count": 4,
                  "time": 9.44,
                  "type": "Bus"
              }
          ],
          "request_id": 6,
          "total_time": 11.44
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Tovarnaya",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 1.78,
                  "type": "Bus"
              },
              {
                  "stop_name": "Universam",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "828",
                  "span_count": 1,
                  "time": 4.92,
                  "type": "Bus"
              }
          ],
          "request_id": 7,
          "total_time": 10.7
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Tovarnaya",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 4,
                  "time": 6.56,
                  "type": "Bus"
              }
          ],
          "request_id": 8,
          "total_time": 8.56
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Tovarnaya",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "635",
                  "span_count": 5,
                  "time": 14.32,
                  "type": "Bus"
              }
          ],
          "request_id": 9,
          "total_time": 16.32
      },
      {
          "items": [
              {
                  "stop_name": "Apteka",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 2.84,
                  "type": "Bus"
              },
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 5.2,
                  "type": "Bus"
              }
          ],
          "request_id": 10,
          "total_time": 12.04
      },
      {
          "error_message": "not found",
          "request_id": 11
      }
  ]
```   
#### Пример 3

Иллюстрации к примеру прилагаются

Ввод
```
  {
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "289",
              "stops": [
                  "Zagorye",
                  "Lipetskaya ulitsa 46",
                  "Lipetskaya ulitsa 40",
                  "Lipetskaya ulitsa 40",
                  "Lipetskaya ulitsa 46",
                  "Moskvorechye",
                  "Zagorye"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.579909,
              "longitude": 37.68372,
              "name": "Zagorye",
              "road_distances": {
                  "Lipetskaya ulitsa 46": 230
              },
              "type": "Stop"
          },
          {
              "latitude": 55.581441,
              "longitude": 37.682205,
              "name": "Lipetskaya ulitsa 46",
              "road_distances": {
                  "Lipetskaya ulitsa 40": 390,
                  "Moskvorechye": 12400
              },
              "type": "Stop"
          },
          {
              "latitude": 55.584496,
              "longitude": 37.679133,
              "name": "Lipetskaya ulitsa 40",
              "road_distances": {
                  "Lipetskaya ulitsa 40": 1090,
                  "Lipetskaya ulitsa 46": 380
              },
              "type": "Stop"
          },
          {
              "latitude": 55.638433,
              "longitude": 37.638433,
              "name": "Moskvorechye",
              "road_distances": {
                  "Zagorye": 10000
              },
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 30,
          "bus_wait_time": 2
      },
      "stat_requests": [
          {
              "id": 1,
              "name": "289",
              "type": "Bus"
          },
          {
              "from": "Zagorye",
              "id": 2,
              "to": "Moskvorechye",
              "type": "Route"
          },
          {
              "from": "Moskvorechye",
              "id": 3,
              "to": "Zagorye",
              "type": "Route"
          },
          {
              "from": "Lipetskaya ulitsa 40",
              "id": 4,
              "to": "Lipetskaya ulitsa 40",
              "type": "Route"
          }
      ]
  }
```   
Вывод
```
  [
      {
          "curvature": 1.63413,
          "request_id": 1,
          "route_length": 24490,
          "stop_count": 7,
          "unique_stop_count": 4
      },
      {
          "items": [
              {
                  "stop_name": "Zagorye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "289",
                  "span_count": 1,
                  "time": 0.46,
                  "type": "Bus"
              },
              {
                  "stop_name": "Lipetskaya ulitsa 46",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "289",
                  "span_count": 1,
                  "time": 24.8,
                  "type": "Bus"
              }
          ],
          "request_id": 2,
          "total_time": 29.26
      },
      {
          "items": [
              {
                  "stop_name": "Moskvorechye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "289",
                  "span_count": 1,
                  "time": 20,
                  "type": "Bus"
              }
          ],
          "request_id": 3,
          "total_time": 22
      },
      {
          "items": [

          ],
          "request_id": 4,
          "total_time": 0
      }
  ]
```   

В файле s12_final_opentest.zip - примеры случайно сгенерированных входных файлов и корректных выводов для них.

Как будет тестироваться ваш код

Программа будет получать на stdin валидные JSON-документы с описанием маршрутов и запросов к справочнику. 

Выводимые в stdout данные будут сравниваться с ожидаемым значением. Вывод вещественных чисел должен осуществляться стандартно — без модификаторов вывода.

Маршруты, выводимые в ответ на запрос Route, должны быть оптимальны по времени. Если маршрутов с минимальным возможным суммарным временем несколько, допускается вывести любой из них: тестирующая система проверяет лишь совпадение времени маршрута с оптимальным и корректность самого маршрута.
