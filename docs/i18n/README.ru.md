<div align="center">

# NanoKit-ESP32

**Официальная экосистема PlatformIO для отладочной платы NanoKit Integrated ESP32**

<p>
<a href="../../README.md">English</a> ·
<a href="README.zh-CN.md">简体中文</a> ·
<a href="README.zh-TW.md">繁體中文</a> ·
<a href="README.ja.md">日本語</a> ·
<a href="README.ko.md">한국어</a> ·
<a href="README.tr.md">Türkçe</a> ·
<strong>Русский</strong> ·
<a href="README.vi.md">Tiếng Việt</a> ·
<a href="README.th.md">ไทย</a> ·
<a href="README.de.md">Deutsch</a> ·
<a href="README.es.md">Español</a> ·
<a href="README.fr.md">Français</a> ·
<a href="README.uk.md">Українська</a> ·
<a href="README.pl.md">Polski</a> ·
<a href="README.pt-BR.md">Português (Brasil)</a> ·
<a href="README.ar.md">العربية</a> ·
<a href="README.fa.md">فارسی</a> ·
<a href="README.id.md">Bahasa Indonesia</a> ·
<a href="README.it.md">Italiano</a> ·
<a href="README.el.md">Ελληνικά</a> ·
<a href="README.ml.md">മലയാളം</a> ·
<a href="README.ur-Latn.md">Roman Urdu</a> ·
<a href="README.ro.md">Română</a>
</p>

<p>
<a href="https://github.com/ProAmineOfficial/NanoKit-ESP32/stargazers"><img src="https://img.shields.io/github/stars/ProAmineOfficial/NanoKit-ESP32?style=social" alt="GitHub stars"></a>
<a href="https://github.com/ProAmineOfficial/NanoKit-ESP32/commits/main"><img src="https://img.shields.io/github/last-commit/ProAmineOfficial/NanoKit-ESP32" alt="Last commit"></a>
<a href="../../LICENSE"><img src="https://img.shields.io/github/license/ProAmineOfficial/NanoKit-ESP32" alt="License"></a>
<a href="https://github.com/ProAmineOfficial/NanoKit-ESP32/actions/workflows/platformio-build.yml"><img src="https://github.com/ProAmineOfficial/NanoKit-ESP32/actions/workflows/platformio-build.yml/badge.svg" alt="PlatformIO Build"></a>
<a href="https://github.com/ProAmineOfficial/NanoKit-ESP32/actions/workflows/repository-index.yml"><img src="https://github.com/ProAmineOfficial/NanoKit-ESP32/actions/workflows/repository-index.yml/badge.svg" alt="Genius Validation"></a>
</p>

<p>
<a href="https://proamine.tech"><img src="https://img.shields.io/badge/Website-proamine.tech-0A84FF" alt="Website proamine.tech"></a>
<a href="https://gitarchitecturediagram.com/ProAmineOfficial/NanoKit-ESP32"><img src="https://img.shields.io/badge/Architecture-Git%20Architecture%20Diagram-7B61FF" alt="Git Architecture Diagram"></a>
<img src="https://img.shields.io/badge/PlatformIO-Arduino%20ESP32-F5822A" alt="PlatformIO Arduino ESP32">
</p>

**Developed by Amine Saoud ibn al-Bashir.**

</div>

> **Примечание о переводе:** эта страница является переводом [английского README](../../README.md). При расхождениях приоритет имеет английский README.

## <img src="../../assets/icons/fluent/page-facing-up.png" width="28" alt=""> Распиновка NanoKit Integrated ESP32: техническое описание

![Распиновка ESP32 (NanoKit Integrated ESP32): техническое описание](../../assets/images/nanokit-integrated-esp32-pinout-v2020-smd-tht.jpg)

Отладочная плата NanoKit Integrated ESP32 — это платформа для разработки встраиваемых систем в области IoT, ИИ, робототехники, образования и профессионального прототипирования. Репозиторий объединяет примеры для PlatformIO, законченные приложения, проекты с открытым исходным кодом, дипломные проекты, ресурсы для симуляции, документацию, статьи, учебные материалы и шаблоны для повторного использования.

Плата NanoKit является частью более широкой аппаратной концепции UMT, однако прошивки в этом репозитории написаны на стандартном Arduino C++ для PlatformIO и используют номера GPIO ESP32, поэтому каждый проект компилируется и запускается непосредственно в PlatformIO.

## <img src="../../assets/icons/fluent/bullseye.png" width="28" alt=""> Миссия репозитория

Это не демонстрационный репозиторий и не репозиторий-шаблон. Это официальный долгосрочный репозиторий экосистемы NanoKit-ESP32 для учебной и профессиональной разработки встраиваемых систем: каждый проект объясняет код, схему подключения, алгоритм, соответствие выводов NanoKit и порядок поиска неисправностей.

Основные правила репозитория, модель его развития, стандарты примеров, требования к документации и критерии готовности (Definition of Done) описаны в [PROJECT_SPECIFICATION.md](../../PROJECT_SPECIFICATION.md).

## <img src="../../assets/icons/fluent/rocket.png" width="28" alt=""> Быстрый старт

1. Установите [VS Code](https://code.visualstudio.com/) с расширением [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) или PlatformIO Core CLI.
2. Клонируйте репозиторий и откройте пример. Начните с [`digital_input_output`](../../examples_on_platformio/digital_input_output/): кнопка, LED и встроенный LED платы.
3. Подключите NanoKit по USB, затем соберите проект, загрузите прошивку и откройте Serial Monitor:

```bash
git clone https://github.com/ProAmineOfficial/NanoKit-ESP32.git
cd NanoKit-ESP32/examples_on_platformio/digital_input_output
pio run --target upload
pio device monitor -b 115200
```

В README каждого проекта перечислены компоненты, таблица выводов NanoKit, порядок подключения и ожидаемый вывод в Serial Monitor.

## <img src="../../assets/icons/fluent/file-folder.png" width="28" alt=""> Структура репозитория

| Путь | Назначение |
|---|---|
| [`examples_on_platformio/`](../../examples_on_platformio/README.md) | Узконаправленные примеры для PlatformIO без пользовательского интерфейса: одна функция платы, один датчик или модуль на пример. |
| [`applications_on_platformio/`](../../applications_on_platformio/README.md) | Законченные приложения с пользовательским интерфейсом, например веб-приложения, которые раздаёт сама плата NanoKit, с кнопками, ползунками и панелями мониторинга. |
| [`open_source_projects/`](../../open_source_projects/README.md) | Более крупные оригинальные проекты NanoKit, такие как NanoKit Drone 4X, а также разделы для сообщества, участников, демонстраций и хакатонов. |
| [`graduation_projects/`](../../graduation_projects/README.md) | Рабочие пространства для бакалаврских, магистерских, инженерных, институтских, итоговых (capstone) и диссертационных проектов. |
| `tutorials/` | Учебные материалы от начального до продвинутого уровня, руководства по проектам и указатели ссылок на видео. |
| `articles/` | Технические статьи о встраиваемых системах, IoT, ИИ, робототехнике, PlatformIO, NanoKit и UMT. |
| `docs/` | Начало работы, установка, аппаратная часть, программное обеспечение, API, устранение неполадок, дорожная карта, архитектура и переводы. |
| `registry/` | Плата NanoKit в виде данных: каждый вывод разъёмов, GPIO, адрес VR и электрическое правило. |
| `tools/` | Автоматизация репозитория: генератор Genius и генератор индекса. |
| `simulator/` | Веб-симулятор, движок симуляции, примеры и ресурсы симулятора. |
| `templates/` | Шаблоны для повторного использования: примеры, приложения, дипломные работы и документация. |
| `assets/` | Общие изображения, иконки, логотипы, баннеры и видео. |

## <img src="../../assets/icons/fluent/gear.png" width="28" alt=""> Аппаратные манифесты и генерируемая документация

Каждый пример, приложение, проект с открытым исходным кодом и дипломный проект описывает своё оборудование один раз — в манифесте `nanokit.yaml`. Команда `python tools/genius/generate.py` проверяет его по реестру платы [`registry/nanokit-esp32.yaml`](../../registry/nanokit-esp32.yaml) и по константам выводов в прошивке, а затем заново генерирует:

- `images/connection-diagram.md` и `images/system-diagram.md` с метками `Pin <NanoKit pin> / GPIO<n>` и стилями Confirmed / Proposed / Blocked,
- таблицу выводов NanoKit в README каждого проекта и таблицу подключений в каждом `docs/Wiring.md`,
- `.genius/VALIDATION_REPORT.md` с перечнем проверенных правил, найденных проблем и подтверждений из кода.

Вывод, занятый внутренней флеш-памятью, выход на выводе, работающем только на вход, сигнал 5 В без преобразования уровней или манифест, расходящийся с кодом, блокируют коммит и pull request. Эталонный проект — [`examples_on_platformio/ultrasonic_distance/`](../../examples_on_platformio/ultrasonic_distance/). Схема манифеста и правила: [`tools/genius/README.md`](../../tools/genius/README.md). Проектное описание: [`docs/software/genius-generator-spec.md`](../../docs/software/genius-generator-spec.md).

## <img src="../../assets/icons/fluent/bar-chart.png" width="28" alt=""> Генерируемые индексы

- [Индекс репозитория](../../REPOSITORY_INDEX.md) кратко описывает основные разделы репозитория и подсчитывает реальные проекты (папки с манифестом).
- [Дерево репозитория](../../REPOSITORY_TREE.md) показывает сгенерированное дерево файлов.
- [Бэклог проектов](../../docs/roadmap/project-backlog.md) перечисляет все запланированные примеры, приложения, учебные материалы и статьи, разбитые на пять волн; его столбец Status и [сводка дорожной карты](../../docs/roadmap/README.md) обновляются автоматически.
- Каждый проект PlatformIO компилируется [рабочим процессом PlatformIO Build](../../.github/workflows/platformio-build.yml) при каждом push и pull request.

После добавления или изменения проекта запустите `python tools/genius/generate.py`, а затем `python tools/update_repository_index.py`. Один раз включите локальный хук командой `git config core.hooksPath .githooks`; рабочий процесс GitHub запускает те же два инструмента при каждом push в `main` и проверяет каждый pull request.

## <img src="../../assets/icons/fluent/deciduous-tree.png" width="28" alt=""> Интерактивное дерево репозитория

Дерево ниже сгенерировано из текущего содержимого репозитория. Щёлкните по его заголовку, чтобы развернуть или свернуть его. Локальный pre-commit хук и рабочий процесс GitHub автоматически обновляют его, когда проекты или файлы добавляются, перемещаются или удаляются.

Полное сгенерированное дерево находится в [REPOSITORY_TREE.md](../../REPOSITORY_TREE.md); в английском README оно показано в виде раскрывающегося списка.

## <img src="../../assets/icons/fluent/building-construction.png" width="28" alt=""> Как устроен репозиторий

```mermaid
flowchart TD
  A[[PROJECT_SPECIFICATION.md]] --> B(Example and project standards)
  R[(registry/nanokit-esp32.yaml)] --> G
  P[/project nanokit.yaml/] --> G(Genius generator)
  S[/firmware pin constants/] --> G
  B --> F(Complete project folder)
  G --> D[[Connection and system diagrams]]
  G --> T[[README and Wiring pin tables]]
  G --> V[[.genius validation report]]
  F --> C(PlatformIO Build workflow)
  D --> F
  T --> F
  V --> F

  classDef document fill:#25243a,stroke:#9ca9ff,stroke-width:1.8px,color:#f6f8ff
  classDef process fill:#142735,stroke:#69d6ff,stroke-width:1.8px,color:#f2fbff
  classDef data fill:#183326,stroke:#7ccd91,stroke-width:1.8px,color:#f4fff6
  class A,D,T,V document
  class B,G,F,C process
  class R,P,S data
  linkStyle default stroke:#7e98aa,stroke-width:1.4px
```

## <img src="../../assets/icons/fluent/helicopter.png" width="28" alt=""> Избранный проект с открытым исходным кодом

[NanoKit Drone 4X (квадрокоптер)](../../open_source_projects/nanokit_drone_4x_quadcopter/README.md) — эталонный проект системы управления полётом Quad-X в этом репозитории, где безопасность стоит на первом месте. Полётный контроллер на NanoKit выполняет контур управления с частотой 250 Гц, микширование моторов Quad-X и защитные блокировки, браузерный Flight Deck подключается по Wi-Fi и WebSocket, а вторая плата NanoKit управляет изолированной полезной нагрузкой — камерой и звуком — без какого-либо доступа к моторам. Проект находится на стадии стендовой разработки: пропеллеры сняты, а выход на моторы удерживается на безопасном минимуме, пока протокол ESC не будет проверен.

## <img src="../../assets/icons/fluent/check-mark-button.png" width="28" alt=""> Контракт качества

Официальное содержимое этого репозитория должно быть готовым к промышленному использованию, учебным, полным и удобным в сопровождении.

- Не оставляйте пустые каталоги в качестве итогового результата.
- Не добавляйте материалы-заглушки, если можно предоставить настоящий учебный материал.
- Развивайте существующую работу, а не перезаписывайте её без необходимости.
- Каждый пример объясняет код, схему подключения, алгоритм, назначение выводов и порядок поиска неисправностей.
- Каждый аппаратный пример документирует номера выводов NanoKit и номера GPIO ESP32 в полной форме: «Pin 36 / GPIO5».

При создании или проверке примеров руководствуйтесь [docs/software/platformio-example-standard.md](../../docs/software/platformio-example-standard.md).

## <img src="../../assets/icons/fluent/hammer-and-wrench.png" width="28" alt=""> Базовая конфигурация PlatformIO

В качестве первой публичной базовой конфигурации проекты PlatformIO используют фреймворк Arduino для ESP32:

```ini
[env:nanokit_esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
```

По мере развития аппаратного пакета специальные определения платы NanoKit могут заменить универсальный профиль платы `esp32dev`.

## <img src="../../assets/icons/fluent/scroll.png" width="28" alt=""> Правило для прошивок

Официальные примеры в этом репозитории должны использовать:

- PlatformIO
- фреймворк Arduino для ESP32
- C++
- документацию в формате Markdown
- обычные номера GPIO ESP32 в исходном коде

Не используйте код UMT SDK или Virtual Register API в примерах PlatformIO.

## <img src="../../assets/icons/fluent/electric-plug.png" width="28" alt=""> Распиновка платы

Используйте [docs/hardware/nanokit-integrated-esp32-pinout.md](../../docs/hardware/nanokit-integrated-esp32-pinout.md) как первоисточник сведений о номерах выводов NanoKit, соответствии GPIO ESP32, электрических характеристиках и предупреждениях.

Каждый аппаратный пример документирует:

```text
NanoKit Pin -> GPIO -> Component Pin -> Description
```

Важные ограничения:

- GPIO34, GPIO35, GPIO36 и GPIO39 работают только на вход.
- GPIO6, GPIO7, GPIO8, GPIO9, GPIO10 и GPIO11 — выводы внутренней SPI флеш-памяти, их использовать нельзя.
- GPIO0 — вывод BOOT.
- GPIO1 и GPIO3 совместно используются с USB Serial.
- EN — это Reset / Enable, а не GPIO.

## <img src="../../assets/icons/fluent/spiral-calendar.png" width="28" alt=""> Дорожная карта

[Бэклог проектов](../../docs/roadmap/project-backlog.md) планирует библиотеку в пять волн: основы работы с платой, датчики и модули, веб-приложения, протоколы и облако, внешние модули. [Сводка дорожной карты](../../docs/roadmap/README.md) показывает, сколько проектов каждой волны уже готово.

## <img src="../../assets/icons/fluent/handshake.png" width="28" alt=""> Участие в разработке

Мы рады вашему вкладу. Сначала прочитайте [CONTRIBUTING.md](../../CONTRIBUTING.md): там описаны стандарты проектов, манифест `nanokit.yaml`, pre-commit хук и лицензирование вносимого вклада.

## <img src="../../assets/icons/fluent/globe-showing-europe-africa.png" width="28" alt=""> Переводы

Этот README доступен на 23 языках через языковую панель вверху страницы. Переводы находятся в [`docs/i18n/`](../../docs/i18n/); если перевод расходится с английским README, приоритет имеет английский README.

## <img src="../../assets/icons/fluent/telephone-receiver.png" width="28" alt=""> Поддержка

- <img src="../../assets/icons/fluent/globe-with-meridians.png" width="20" alt=""> Веб-сайт: [https://proamine.tech](https://proamine.tech)
- <img src="../../assets/icons/fluent/puzzle-piece.png" width="20" alt=""> Git Architecture Diagram: [https://gitarchitecturediagram.com](https://gitarchitecturediagram.com/ProAmineOfficial/NanoKit-ESP32)
- <img src="../../assets/icons/fluent/e-mail.png" width="20" alt=""> Эл. почта: `technology@proamine.tech`
- <img src="../../assets/icons/fluent/movie-camera.png" width="20" alt=""> TikTok: [@pro_amine](https://www.tiktok.com/@pro_amine)
- <img src="../../assets/icons/fluent/movie-camera.png" width="20" alt=""> YouTube: [@pro_amine](https://www.youtube.com/@pro_amine)

## <img src="../../assets/icons/fluent/locked-with-key.png" width="28" alt=""> Лицензия

NanoKit-ESP32 распространяется по лицензии [GNU Affero General Public License v3.0 only](../../LICENSE) (`AGPL-3.0-only`) с дополнительными условиями по разделу 7, перечисленными в [NOTICE](../../NOTICE):

- **Указание авторства:** каждая копия или изменённая версия сохраняет строку «Developed by Amine Saoud ibn al-Bashir.» и указывает, что она основана на NanoKit-ESP32 от Amine Saoud ibn al-Bashir (Pro_Amine LLC).
- **Изменённые версии** помечаются как отличающиеся от оригинала и не выдаются за официальный репозиторий NanoKit-ESP32.
- **Названия:** никакие права на названия NanoKit, Pro_Amine или UMT и на их логотипы не предоставляются; распространяемая версия использует собственное название.
- **Использование по сети:** если вы запускаете изменённую версию как сетевой сервис, AGPL обязывает вас предоставить её пользователям исходный код этой версии.

На этих условиях вы можете свободно изучать, использовать и изменять этот код для обучения, преподавания, исследований и собственных проектов. Версии, опубликованные до перехода на AGPL, распространялись по лицензии Apache License 2.0. Значки Fluent Emoji в `assets/icons/fluent/` — © Microsoft Corporation, распространяются по лицензии MIT License.

<div align="center">

**Developed by Amine Saoud ibn al-Bashir · © 2026 Pro_Amine LLC**

</div>
