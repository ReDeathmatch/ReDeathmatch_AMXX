<p align="center">
    <a href="https://github.com/wopox1337/ReDeathmatch">
        <img
            width="500px"
            alt="Gun logo"
            src="https://user-images.githubusercontent.com/18553678/233882657-0ee4d8ea-2492-4af7-8db5-32430689c131.png"
        >
    </a>
</p>

<p align="center">
    Плагины AMXModX для обеспечения геймплея Deathmatch в <a href="https://store.steampowered.com/app/10/CounterStrike/">Counter-Strike 1.6</a>, оптимизированные для работы с <a href="https://github.com/s1lentq/ReGameDLL_CS">ReGameDLL_CS</a>.
</p>

<p align="center">
    <a href="https://github.com/wopox1337/ReDeathmatch/releases/latest">
        <img
            src="https://img.shields.io/github/downloads/wopox1337/ReDeathmatch/total?label=Download%40latest&style=flat-square&logo=github&logoColor=white"
            alt="Build status"
        >
    </a>
    <a href="https://github.com/wopox1337/ReDeathmatch/actions">
        <img
            src="https://img.shields.io/github/actions/workflow/status/wopox1337/ReDeathmatch/CI.yml?branch=master&style=flat-square&logo=github&logoColor=white"
            alt="Build status"
        >
    </a>
    <a href="https://github.com/wopox1337/ReDeathmatch/releases">
        <img
            src="https://img.shields.io/github/v/release/wopox1337/ReDeathmatch?include_prereleases&style=flat-square&logo=github&logoColor=white"
            alt="Release"
        >
    </a>
    <a href="https://www.amxmodx.org/downloads-new.php">
        <img
            src="https://img.shields.io/badge/AMXModX-%3E%3D1.9.0-blue?style=flat-square"
            alt="AMXModX dependency"
        >
        </a>
</p>

## О модификации
Мод представляет собой полностью переписанную реализацию [CSDM ReAPI](https://github.com/wopox1337/CSDM-ReAPI) для замены устаревшего кода.

Мод сделан с оглядкой на успешный опыт [CSDM 2.1.2 by BAILOPAN](https://www.bailopan.net/csdm), но с использованием современных возможностей нового [ReGameDLL_CS](https://github.com/s1lentq/ReGameDLL_CS).

Многие функции уже давно собраны и оптимизированы для работы непосредственно в ReGameDLL_CS, мод теперь только переключает настройки игры и предоставляет удобный способ управления.

## Особенности
- Сохранение настроек игры (CVars);
- Режимы раунда (*NEW*);
- Горячая перезагрузка конфигурации;
- Рандомизированный, предустановленный спавн (можно добавлять новые точки спавна, предустановки спавна);
- Защита от спавна (настраивается по времени и рендеру игрока);
- Интерактивный редактор спавна;
- Настраиваемые меню оружия;
- Командный Deathmatch, а также FFA (Free-for-all Deathmatch);
- Большие части оптимизированы в ReGameDLL_CS;
- Поддержка нескольких языков;
- Поддержка экстраконфигов:
    - Для отдельной карты (`redm/extraconfigs/de_dust2.json`);
    - Для префикса карты (`redm/extraconfigs/prefix_aim.json`).
- Counter-Strike: Condition Zero поддерживается из коробки;
- Поддержка установки группировки для спавнов;
- Возможность использовать мод в качестве основы для разработки других модов (например, `GunGame`);

## Требования
- Установлен HLDS;
- Установлен [ReGameDLL_CS](https://github.com/s1lentq/ReGameDLL_CS);
- Установлен AMXModX ([`v1.9`](https://www.amxmodx.org/downloads-new.php) или [`v1.10`](https://www.amxmodx.org/downloads-new.php?branch=master));
- Установлен [ReAPI](https://github.com/s1lentq/reapi) модуль amxx;
      
## Установка
Проверьте вики [Установка](https://github.com/wopox1337/ReDeathmatch/wiki/Установка)

## Обновление
- Поместите новые плагины и lang-файлы (`plugins/*.amxx` и `data/lang/*.txt`) в папку `amxmodx/` на сервере HLDS;
- Перезапустите сервер (команда `restart` или смените карту);
- Убедитесь, что версии плагинов актуальны с помощью команды `amxx list`.

## Загрузки
- [Релизные билды](https://github.com/wopox1337/ReDeathmatch/releases)
- [Промежуточные билды](https://github.com/wopox1337/ReDeathmatch/actions/workflows/build.yml)
