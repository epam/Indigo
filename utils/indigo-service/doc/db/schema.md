
## База данных indigo-service

Мы используем [PostgreSQL 9.4](http://www.postgresql.org/docs/9.4/static/release-9-4.html) с установленным картриждем [Bingo](http://lifescience.opensource.epam.com/bingo/bingo-postgres.html).

Все действия сервис выполняет в PostgreSQL schema ```indigoservice``` под одноимённым пользователем.

### Таблицы

Набор стурктур, загружаемый пользователем в формате [SD-файла](https://en.wikipedia.org/wiki/Chemical_table_file#SDF) мы называем библиотекой (*library*).

#### ```library_metadata```
Каждая запись в данной таблице соотвествует одной библиотеке сервиса, сами данные библиотеки хранятся в таблице ```bingo_structures_XXX``` (см. ниже).

- **library\_id** *varchar(36) primary key*

уникальный ID; вычисляется на основании имени, указанного пользователем при создании библиотеки

- **service_data** *jsonb*

сервисные данные о библиотеке: имя библиотеки, время создания, количество структур и пр.

- **metadata** *jsonb*

дополнительные данные от пользователя

- **index_data** *jsonb*

поисковые данные

#### ```bingo_structures_XXX```
Таблицы данного типа создаются сервисом (c помощью API метода ```POST /libraries```), одной библиотеке соответствует одна таблица. ```XXX``` – это **library_id** из таблицы ```library_metadata``` (нормализованный с помощью регулярного выражения ```s/-/_/g```). Одна запись соответствует одной структуре загруженного файла. Загрузка данных в таблицу осуществляется посредством API метода ```POST /libraries/{library_id}/uploads```. Имена полей намеренно выбраны короткими в целях оптимизации производительности поисковых запросов.

- **s** *serial*

внутренний ID структуры

- **m** *bytea*

структура в формате [Molfile](https://en.wikipedia.org/wiki/Chemical_table_file#Molfile)

- **p** *jsonb*

список свойств извлечённый из [SD-файла](https://en.wikipedia.org/wiki/Chemical_table_file#SDF)

### Индексы

#### ```bingo_idx_YYY```

Для каждой таблицы  ```bingo_structures_XXX``` создаётся индекс  ```bingo_idx_YYY```, где ```YYY``` – результат хэширования имени таблицы. Полностью запрос для создания индекса выглядит следующим образом:

```
"create index {0} on {1} using bingo_idx (m bingo.bmolecule) with (IGNORE_STEREOCENTER_ERRORS=1,IGNORE_CISTRANS_ERRORS=1,FP_TAU_SIZE=0)".format(index_name, table_name)
```
