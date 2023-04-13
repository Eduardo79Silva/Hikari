-- premake5.lua
workspace "Majin"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Majin"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "WalnutExternal.lua"
include "Majin"