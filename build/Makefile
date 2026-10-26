# LEGO Indiana Jones. Rules: engine windows/build/game.mk; names: ../game.conf.
ENGINE ?= $(abspath ../engine)
GAME_SRCS := $(wildcard $(abspath ../runtime/src)/*.c)
# LEGOIndy.exe (GOG): entry RVA, six sections.
GAME_CFLAGS := -DPE_ENTRY_RVA=0x003132ACu -DPE_SECTION_COUNT=6u
include $(ENGINE)/windows/build/game.mk
