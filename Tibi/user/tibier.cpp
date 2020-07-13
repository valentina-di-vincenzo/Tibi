/*
 * Copyright (c) 2020 Valentina Di Vincenzo ( hello@valentina-divincenzo.com )
 *  This file is part of Tibi which is released under the GNU General Public License, version 3.0.
 * See file LICENSE or go to http://www.gnu.org/licenses/ for full license details.
 *
 */

#include "tibier.h"


bool Tibier::operator==(const Tibier &tibier) {

    if(this->id == tibier.id) return true;
    return false;

}


