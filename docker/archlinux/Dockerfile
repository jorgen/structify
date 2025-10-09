FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
  pacman -S --noconfirm base-devel clang libc++ cmake ninja git vim python3

ENV CXX=clang++
ENV CC=clang

RUN git clone --depth=1 https://github.com/llvm/llvm-project && \
    cd llvm-project && \
    mkdir build; \
    cmake -G Ninja -S runtimes -B build -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" -DLLVM_USE_SANITIZER=MemoryWithOrigins  -DCMAKE_INSTALL_PREFIX=/opt && \
    ninja -C build cxx cxxabi && \
    ninja -C build install-cxx install-cxxabi

ENV MSAN_CFLAGS="-std=c++20 -fsanitize=memory -nostdinc++ -I/opt/include -I/opt/include/c++/v1"
ENV MSAN_LFLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/lib -lc++abi -Wl,-rpath,/opt/lib"
