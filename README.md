# Функциональные требования

## 1. Действия с файловой системой
   1.1. Создание новых папок в текущей папке панели; 
   
   ГОТОВО
   
   1.2.  Отображение содержимого папки с выбором способа сортировки: имя файла, тип, размер, дата модификации;
   
   ГОТОВО ЧАСТИЧНО (сортировка применяется на обе панели, не отображаются значки сортировки)
   
   1.3. Вызов приложений для создания/просмотра/редактирования файлов в форматах TXT, PDF, HTML  и MsWord;
   
   ГОТОВО
   
   1.4. Переименование файлов и папок;
   
   ГОТОВО
   
   1.5. Одиночное/множественное выделение файлов и/или папок, снятие выделения;
   
   ГОТОВО (НО МЕСТАМИ РАБОТАЕТ КОРЯВО)
   
   1.6. Создание информационного портрета (индексирование документа в форматах TXT, PDF, HTML и MsWord) с предварительным просмотром в БД или формат XML для выделенных элементов или элемента под курсором. С использованием для индексирования семантического web-сервиса;
   
   1.7. Копирование/перемещение информационного портрета из формата XML в БД для выделенных элементов или элемента под курсором.
   
   ГОТОВО
   
   1.8. Копирование выделенных элементов или элемента под курсором в папку другой панели;
   
   ГОТОВО
   
   1.9. Перемещение выделенных элементов или элемента под курсором в папку другой панели;
   
   ГОТОВО
   
   1.10. Удаление выделенных элементов или элемента под курсором.

  ГОТОВО

## 2. Действия с базой данных ИП/документов
   2.1. Создание новых коллекций в текущей коллекции панели;
   
   2.2. Отображение содержимого коллекции БД с выбором способа сортировки: идентификатор, название, размер, дата модификации, автор;
   
   2.3. Просмотр информационного портрета (свойства, текст, ключевые слова и шинглы с сортировкой по значимости ↑↓ и по алфавиту ↑↓, распределение весов top N слов и шинглов в графическом виде, структура, исходный текст) из БД или формата XML;
   
   2.4. Редактирование информационных портретов (пока не делаем);
   
   2.5. Переименование коллекций и портретов;
   
   2.6. Одиночное/множественное выделение портретов и/или коллекций, снятие выделения;
   
   2.7. Копирование/перемещение информационного портрета из БД в формат XML для выделенных элементов или элемента под курсором.
   
   2.8. Копирование выделенных элементов или элемента под курсором в коллекцию другой панели БД;
   
   2.9. Перемещение выделенных элементов или элемента под курсором в коллекцию другой панели БД;
   
   2.10. Удаление выделенных элементов или элемента под курсором.

## 3. Семантические функции (все с использованием семантических web-сервисов)
   3.1. Сравнение двух информационных портретов, заданных положениями курсоров в двух панелях;
   
   3.2. Классификация информационных портретов в одной панели по классам, заданным информационными портретами в другой панели;
   
   3.3. Кластеризация информационных портретов в текущей панели на заданное количество кластеров;
   
   3.4. Поиск информационных портретов, максимально/минимально похожих на портрет под курсором или совокупность выделенных портретов в одной панели среди портретов другой панели (возможно, с учетом вложенных);
   
   3.5. Поиск информационных портретов среди портретов текущей коллекции панели (возможно, с учетом вложенных), максимально/минимально соответствующих текстовому запросу;
   
   3.6. Анализ покрытия информационными портретами в одной панели тематических разделов (классов), заданных информационными портретами в другой панели (возможно, с учетом вложенных как в одной, так и в другой панели);
   
   3.7. Поиск лакун (недостающих в текущей панели информационных портретов, имеющихся в другой, заданной коллекции) для тематических разделов (классов), заданных информационными портретами в другой панели (возможно, с учетом вложенных в другой панели);
   
   3.8. Вызов веб-страницы интеллектуальной системы поиска для просмотра документа с семантической навигацией;
   
   3.9. Расчет совокупного информационного портрета выборке выделенных в текущей панели портретов;
   
   3.10. Отображение наиболее значимых терминов и шинглов для заданного запроса. Уточнение и расширение запроса терминами и шинглами из полученных списков;
   
   3.11. Генерация диаграммы/облака наиболее взаимосвязанных терминов по заданному списку (портрет документа/совокупный портрет выбор

## 4.	Вспомогательные функции
   4.1.	Извлечение из документа картинок и сопроводительного текста к ним.
