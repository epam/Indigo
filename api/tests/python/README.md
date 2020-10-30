# Тестирование Indigo
Тесты Indigo написаны на общем подмножестве Python 2 и 3. Мы также тестируем ими и обертки для Java и .NET при помощи Jython и IronPython. Тесты работают на всех поддерживаемых операционных системах (Windows x64 и x86, Linux x64 и x86, Mac OS X 10.7+ x64 и x86). Чтобы обеспечить подобную универсальность, надо следовать нескольким простым правилам при написании тестов.

## Правила создания тестов:

1. Тесты должны быть независимы. Движок запуска тестов не гарантирует порядка их выполнения, а также позволяет запускать отдельные тесты. Не стоит надеяться на наличие директории `foo` в тесте 2, если Вы создаёте её в тесте 1.
2. Тесты должны использовать только общее подмножество Python 2 и 3. В первую очередь это касается вывода с использованием `print`. Всегда используйте `print` со скобками и передачей только одного, явно отформатированного аргумента: `print('{0} {1} {2}'.format('a', 'b', 'c'))`. Это необходимо, поскольку
 * В Python 2 `print` это ключевое слово, и `print('a', 'b', 'c')` выведет кортеж `('a', 'b', 'c')`. 
 * В Python 3 `print` это функция, и `print('a', 'b', 'c')` выведет отформатированную строку `a b c`. 
 

## Пример теста Indigo

```
import sys
sys.path.append('../../common') # Директория с env_indigo
from env_indigo import * # Загружаем Indigo

indigo = Indigo()
m = indigo.loadMolecule('C')
print("SMILES: {0}".format(m.smiles())) # В Python 3 print это функция, так что необходимо использовать скобки, также надо явно форматировать строку, передавая print всего один аргумент
try:
    m.getAtom(2)
except IndigoException as e:
    print("Error: {0}".format(getIndigoExceptionText(e))) # Вывод исключений должен использовать getIndigoExceptionText
```

* Импорт env_indigo обеспечивает загрузку indigo-python, indigo-java или indigo-dotnet из директорий `../indigo/api`, `../indigo/dist`, или `indigo-tests/dist/`. Это равносильно импорту Indigo, IndigoException, IndigoInchi, IndigoRenderer, Bingo, BingoException, BingoObject.

## Пример теста IndigoRenderer

```
import sys
sys.path.append('../../common')
from env_indigo import *
from rendering import *

if not os.path.exists(joinPath("out")):
    os.makedirs(joinPath("out"))

indigo = Indigo()
renderer = IndigoRenderer(indigo)

mol = indigo.loadMolecule("CCNNCN")

# Тестирование рисования SVG
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPath("out/rsite_highlighted.svg"))
print(checkImageSimilarity('rsite_highlighted.svg'))

# Тестирование рисования PNG
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPath('out/rsite_highlighted.png'))
print(checkImageSimilarity('rsite_highlighted.png'))
```
* Функция checkImageSimilarity() сравнивает результат, расположенный в директории `indigo-tests/api/tests/rendering/out` с эталоном, расположенным в директории `indigo-tests/api/tests/rendering/ref/{%os%}`, где {%os%} это `win`, `linux` или `mac` в зависимости от используемой операционной системы. Это необходимо, поскольку отрисовка шрифтов осуществляется с использованием системных библиотек и различается на разных системах.

## Полезные функции из env_indigo
* `isJython()` — возвращает `True`, если тест исполняется indigo-java
* `isIronPython()` — возвращает `True`, если тест исполняется indigo-dotnet
* `getPlatform()` — возвращает `win`, `linux` или `mac` в зависимости от используемой операционной системы
* `getIndigoExceptionTest(IndigoException)` — возвращает текст пойманного исключения `IndigoException` в универсальном формате, вне зависимости от того, используется ли Python, Java или .NET. Полезно при проверке возвращаемого сообщения об ошибке.
* `joinPath(*args)` — возвращает абсолютное значение пути, относительного вызывающего файла. Например, `joinPath('molecules/helma_smi')`, вызванный в тесте `indigo-tests/api/tests/basic/basic.py`, вернет путь `indigo-tests/api/tests/basic/molecules/helma.smi`