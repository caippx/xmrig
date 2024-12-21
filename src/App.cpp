/* XMRig
 * Copyright 2010     
 * Copyright 2012-2014 
 * Copyright 2014     
 * Copyright 2014-2016 
 * Copyright 2016      
 * Copyright 2017-2018 
 * Copyright 2018     
 * Copyright 2018-2024 
 * Copyright 2016-2024 
 * SB weiruan bie feng wo hao
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <uv.h>


#include "App.h"
#include "backend/cpu/Cpu.h"
#include "base/io/Console.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/io/Signals.h"
#include "base/kernel/Platform.h"
#include "core/config/Config.h"
#include "core/Controller.h"
#include "Summary.h"
#include "version.h"


xmrig::App::App(Process *process)
{
    m_controller = std::make_shared<Controller>(process);
}


xmrig::App::~App()
{
    Cpu::release();
}


int xmrig::App::exec()
{
    if (!m_controller->isReady()) {
        LOG_EMERG("no valid configuration");

        return 2;
    }

    int rc = 0;
    if (background(rc)) {
        return rc;
    }

    m_signals = std::make_shared<Signals>(this);

    rc = m_controller->init();
    if (rc != 0) {
        return rc;
    }

    if (!m_controller->isBackground()) {
        m_console = std::make_shared<Console>(this);
    }

    Summary::print(m_controller.get());

    if (m_controller->config()->isDryRun()) {
        LOG_NOTICE("%s " WHITE_BOLD("OK"), Tags::config());

        return 0;
    }

    m_controller->start();

    rc = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    uv_loop_close(uv_default_loop());

    return rc;
}


void xmrig::App::onConsoleCommand(char command)
{
    if (command == 3) {
        LOG_WARN("%s " YELLOW("tuichu get, exiting"), Tags::signal());
        close();
    }
    else {
        m_controller->execCommand(command);
    }
}


void xmrig::App::onSignal(int signum)
{
    switch (signum)
    {
    case SIGHUP:
    case SIGTERM:
    case SIGINT:
        return close();

    default:
        break;
    }
}


void xmrig::App::close()
{
    m_signals.reset();
    m_console.reset();

    m_controller->stop();

    Log::destroy();
}
