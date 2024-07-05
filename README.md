## Async-profiler collector

Непрерывное wall-clock профилирование метода JVM-приложения.

### Запуск

```
collector [-i <interval>] -p <java_pid> -m <full_method_name> -b <profiler_binary_path>  
  -i               интервал профилирования в милисекундах 
  -p | --pid       id java-процесса
  -m | --method    полное название профилируемого метода
  -b | --binary    путь к исполняемоуму файлу async-profiler  

Пример:
collector -p 1234 -b async-profiler/build/bin/asprof -m org/yznal/profilingdemo/Common.doStuff -i 10
```

### Сборка

Сборка осуществяется с помощью CMake
```bash
cd build/
cmake ..
make collector
```

### Зависимости

```
async-profiler - https://github.com/async-profiler/async-profiler
prometheus-cpp - https://github.com/jupp0r/prometheus-cpp
```

### Формат метрик

Метрики поставляются в prometheus-формате, доступны на порту `8082` по пути `/metrics`  
Формат счетчиков:  

```
{ host, caller, callee, depth} -> value 
host    - хост
caller  - название фрейма корневого наблюдаемого метода
callee  - название дочернего фрейма
depth   - глубина стека, относительно корневого метода
value   - количество вызовов методов
```
  
Ориентированный интервал сбора метрик - 15 секунд.


