/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#include "progressbar.h"
#include <iostream>

ProgressBar::ProgressBar()
{
    setMinimum(0);
    setMaximum(0);
    setMaximumWidth(150);
    QProgressBar::hide();
}

bool ProgressBar::empty()
{
    return m_process_info.empty();
}

void ProgressBar::hide(std::string process)
{
    std::string tool = "";
    for (unsigned int i = 0; i < m_process_info.size(); i++) {
        if (m_process_info[i].find(process) != std::string::npos) {
            m_process_info.erase(m_process_info.begin() + i);

            if (m_process_info.empty()) {
                QProgressBar::hide();
                break;
            }

            continue;
        }
        tool += m_process_info[i];
        if (i != m_process_info.size() - 1) {
            tool += "\n";
        }
    }

    if (tool[tool.length() - 1] == '\n')
        tool.erase(tool.length() - 1);
    setToolTip(tool.c_str());
}

void ProgressBar::show(std::string process)
{
    for (unsigned int i = 0; i < m_process_info.size(); i++) {
        if (m_process_info[i].find(process) != std::string::npos) {
            return;
        }
    }
    m_process_info.push_back(process);
    std::string tool = toolTip().toStdString();
    if (!tool.empty()) {
        tool += "\n";
    }
    tool += process;
    setToolTip(tool.c_str());
    QProgressBar::show();
}
