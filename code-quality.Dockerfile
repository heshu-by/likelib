# Create docker environment for code cuality
FROM likelib2_build as code_quality

# copy project file from previous build
ENV PROJECT_SOURCE_DIR /project
COPY from=code_quality ${PROJECT_SOURCE_DIR}

WORKDIR ${PROJECT_SOURCE_DIR}/src

RUN clang-tidy node/main.cpp -- -I/opt/vcpkg/installed/x64-linux/include -I${PWD} -std=c++17

