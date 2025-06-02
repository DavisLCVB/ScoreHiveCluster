FROM ubuntu:22.04

# Evitar prompts interactivos
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependencias del sistema y librerías
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    gcc-12 \
    g++-12 \
    openmpi-bin \
    openmpi-common \
    libopenmpi-dev \
    openssh-server \
    openssh-client \
    net-tools \
    iputils-ping \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Configurar GCC 12 como compilador por defecto
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100

# Instalar spdlog
RUN cd /tmp && \
    git clone https://github.com/gabime/spdlog.git && \
    cd spdlog && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_COMPILER=g++-12 && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf /tmp/spdlog

# Instalar nlohmann_json
RUN cd /tmp && \
    git clone https://github.com/nlohmann/json.git && \
    cd json && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DJSON_BuildTests=OFF -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_COMPILER=g++-12 && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf /tmp/json

# Actualizar cache de librerías
RUN ldconfig

# Configurar SSH para comunicación MPI interna
RUN mkdir /var/run/sshd
RUN echo 'root:mpipassword' | chpasswd
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/#PubkeyAuthentication yes/PubkeyAuthentication yes/' /etc/ssh/sshd_config

# Generar llaves SSH para comunicación interna MPI
RUN ssh-keygen -t rsa -f /root/.ssh/id_rsa -N ''
RUN cp /root/.ssh/id_rsa.pub /root/.ssh/authorized_keys
RUN chmod 600 /root/.ssh/authorized_keys


# Configurar SSH para MPI (sin verificación de host)
RUN echo "Host *" >> /root/.ssh/config && \
    echo "    StrictHostKeyChecking no" >> /root/.ssh/config && \
    echo "    UserKnownHostsFile=/dev/null" >> /root/.ssh/config

# Crear directorio de aplicación
WORKDIR /app

# Copiar código fuente
COPY . /app/

# Compilar aplicación (Debug por defecto) - Usar C++20
RUN mkdir -p build/debug && \
    cd build/debug && \
    cmake ../.. -DCMAKE_BUILD_TYPE=Debug \
                -DCMAKE_CXX_COMPILER=g++-12 \
                -DCMAKE_C_COMPILER=gcc-12 \
                -DCMAKE_CXX_STANDARD=20 \
                -DCMAKE_CXX_STANDARD_REQUIRED=ON \
                -DCMAKE_CXX_FLAGS="-std=c++20" && \
    make -j$(nproc)

# Compilar versión Release también - Usar C++20
RUN mkdir -p build/release && \
    cd build/release && \
    cmake ../.. -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_CXX_COMPILER=g++-12 \
                -DCMAKE_C_COMPILER=gcc-12 \
                -DCMAKE_CXX_STANDARD=20 \
                -DCMAKE_CXX_STANDARD_REQUIRED=ON \
                -DCMAKE_CXX_FLAGS="-std=c++20" && \
    make -j$(nproc)

# Configurar variables de entorno para MPI
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

# Crear script de inicio
RUN echo '#!/bin/bash\n\
/usr/sbin/sshd\n\
if [ "$(hostname)" = "mpi-master" ]; then\n\
    sleep 10\n\
    if [ "$DEBUG" = "1" ]; then\n\
        env DEBUG=1 mpirun  --allow-run-as-root --hostfile /app/hostfile  -n 6 /app/build/debug/ScoreHiveCluster\n\
    else\n\
        mpirun  --allow-run-as-root --hostfile /app/hostfile -n 6 /app/build/release/ScoreHiveCluster\n\
    fi\n\
else\n\
    tail -f /dev/null\n\
fi' > /app/start.sh && chmod +x /app/start.sh

# Exponer puerto del servidor
EXPOSE 8080

# Ejecutar script de inicio
CMD ["/app/start.sh"]