#pragma once

#include "args.hpp"

void compile_project();
void run(Args::Run const &args);
void build_project(bool fork = false);
void update_cppweb(Args::Update const &args, bool nobuild = false);
void create_project(Args::Create const &args);
void kill();
