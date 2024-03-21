# CADEV

Проект пока не готовый.

Так все dll файлы OpenCascade отсутствуют на GitHub, их необходимо скачать вручную:

https://drive.google.com/file/d/1Qw5F6beNd-7M-x2ZYZD0Q6vtrFRw25_H/view?usp=sharing

Если при запуске проекта высвечивается ошибка об отсутсвии TKernel.dll, TKOpenGL.dll, TKPrim.dll и TKService.dll, то необходимо перейти в свойства проекта "Проект -> Свойства", во вкладке "Свойства конфигурации -> Отладка" в поле "Окружение" добавить следубщую строку:

PATH=$(ProjectDir)OpenCascade\bin;$(ProjectDir)OpenCascade\tbb2021.5-vc14-x64\bin;$(ProjectDir)OpenCascade\openvr-1.14.15-64\bin\win64;$(ProjectDir)OpenCascade\freeimage-3.17.0-vc14-64\bin;$(ProjectDir)OpenCascade\ffmpeg-3.3.4-64\bin;$(ProjectDir)OpenCascade\jemalloc-vc14-64\bin;$(ProjectDir)OpenCascade\freetype-2.5.5-vc14-64\bin;%PATH%

Основано на проекте:

https://github.com/eryar/OcctImgui
