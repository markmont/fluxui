/* Copyright (c) 2012 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. */

#ifndef HANDLERS_H_
#define HANDLERS_H_

#define MAX_PARAMS 4

typedef int (*HandleFunc)(int num_params, char** params, char** output);

int HandleRunTest(int num_params, char** params, char** output);
int HandleConnectToFlux(int num_params, char** params, char** output);
int HandleDisconnectFromFlux(int num_params, char** params, char** output);

#endif /* HANDLERS_H_ */
