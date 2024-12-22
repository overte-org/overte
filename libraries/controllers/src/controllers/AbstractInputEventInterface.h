//  AbstractInputEventInterface.h
//  libraries/controllers/src/controllers
//
//  Created by dr Karol Suprynowicz on 2024.07.04.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef overte_AbstractInputEventInterface_h
#define overte_AbstractInputEventInterface_h

#include <QEvent>

/// Used by the libraries to post events into overlay UI.
class AbstractInputEventInterface {
public:
    virtual void postEventToOverlayUI(QEvent *event) = 0;
};

#endif // overte_AbstractInputEventInterface_h
