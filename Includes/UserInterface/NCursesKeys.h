#pragma once

/* Copyright (c) 2020 [Rick de Bondt] - NetworkingHeaders.h
 *
 * This file contains constants with keycodes that can be used to catch keys.
 *
 **/

static constexpr unsigned int cKeyTab{9};

// This starts the keypad scancode thing
static constexpr unsigned int cKeyEsc{27};
static constexpr unsigned int cKeypadArrowCombo{79};
static constexpr unsigned int cKeypadCenterCombo{91};

static constexpr unsigned int cKeypadDown{114};
static constexpr unsigned int cKeypadLeft{116};
static constexpr unsigned int cKeypadRight{118};
static constexpr unsigned int cKeypadUp{120};
static constexpr unsigned int cKeypadCenter{69};

// These keycodes match Windows keycodes so that works as well
static constexpr unsigned int cCombinedKeypadUp{60610};
static constexpr unsigned int cCombinedKeypadDown{60616};
static constexpr unsigned int cCombinedKeypadLeft{60612};
static constexpr unsigned int cCombinedKeypadRight{60614};
static constexpr unsigned int cCombinedKeypadCenter{60613};
