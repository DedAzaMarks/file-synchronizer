# file - synchronizer
Синхронизация набора файлов по сети между удаленными клиентами
Для компилирования и работы программы необходима установка библиотек `PocoNet` и `PocoFoundation`  
https://github.com/pocoproject/poco  
**Требуются**:
* CMake 3.5 or newer
* A C++14 compiler (Visual C++ 2015, GCC 5.0, Clang 3.4, or newer)
* OpenSSL headers and libraries (optional, but recommended)

**Установка Poco**:   
```bash
git clone -b master https://github.com/pocoproject/poco.git
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install
```
По умолчанию установка происходит в папке `/usr/local/`. (*Возможны проблемы с подключением библиотек. В таком случае неоходимо переместить библиотеки Poco в папку `/usr/lib/`.*)
```bash
mv /usr/local/lib/libPoco* /usr/lib/
```
<hr>

**Установка**:
```bash
git clone https://github.com/DedAzaMarks/file-synchronizer.git
cd file-synchronizer
make
```
Будут скомпилированы два исполняемых файла
* sync в аргументах командной строки требует `IP` и `<file>`, а также опционально размер блока (по умолчанию 2048)
* server не требует аргументов командной строки
