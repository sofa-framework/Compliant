/******************************************************************************
*                 SOFA, Simulation Open-Framework Architecture                *
*                    (c) 2006 INRIA, USTL, UJF, CNRS, MGH                     *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_CONFIGURATIONSETTING_CompliantAttachBUTTON_H
#define SOFA_COMPONENT_CONFIGURATIONSETTING_CompliantAttachBUTTON_H

#include <Compliant/config.h>
#include <sofa/core/objectmodel/ConfigurationSetting.h>
#include <SofaGraphComponent/MouseButtonSetting.h>

#include <sofa/type/RGBAColor.h>

namespace sofa
{

namespace component
{

namespace configurationsetting
{

/**
 * Mouse using multi mapping based modelisation rather than interaction force field (not only for compliant)
 */
class SOFA_Compliant_API CompliantAttachButtonSetting: public MouseButtonSetting
{
public:
    SOFA_CLASS(CompliantAttachButtonSetting,MouseButtonSetting);
protected:
    CompliantAttachButtonSetting();
public:
    std::string getOperationType() override {return  "CompliantAttach";}
    Data<SReal> compliance; ///< Compliance of the manipulator. 0 is rigid, the bigger the softer. Negative values make no sense.
    Data<bool> isCompliance; ///< Is the mouse interaction treated as a compliance? (otherwise as a stiffness)
    Data<SReal> arrowSize;
    Data<sofa::type::RGBAColor> color;
    Data<bool> visualmodel;
};

}

}

}
#endif
