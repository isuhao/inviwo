/**********************************************************************
 * Copyright (C) 2012-2013 Scientific Visualization Group - Link�ping University
 * All Rights Reserved.
 * 
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * No part of this software may be reproduced or transmitted in any
 * form or by any means including photocopying or recording without
 * written permission of the copyright owner.
 *
 * Primary author : Erik Sund�n
 *
 **********************************************************************/

#ifndef IVW_VOLUMEREPRESENTATION_H
#define IVW_VOLUMEREPRESENTATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/datarepresentation.h>

namespace inviwo {

class IVW_CORE_API VolumeRepresentation : public DataRepresentation {

public:
    /// struct volume borders
    struct VolumeBorders{
        uvec3 llf;
        uvec3 urb;
        size_t numVoxels;
        bool hasBorder;

        VolumeBorders() : llf(uvec3(0,0,0)), urb(uvec3(0,0,0)), numVoxels(0), hasBorder(false){}
        VolumeBorders(size_t front, size_t back, size_t left, size_t right, size_t lower, size_t upper) : llf(uvec3(front, left, lower)), urb(uvec3(back, right, upper)){}
        VolumeBorders(const uvec3& llfBorder, const uvec3& urbBorder) : llf(llfBorder), urb(urbBorder){}
        bool operator== (const VolumeBorders &vb) const{ return (llf == vb.llf && urb == vb.urb);}
        bool operator!= (const VolumeBorders &vb) const{ return (llf != vb.llf || urb != vb.urb);}
    };

    VolumeRepresentation(uvec3 dimension);
    VolumeRepresentation(uvec3 dimension, const DataFormatBase* format);
    VolumeRepresentation(uvec3 dimension, const DataFormatBase* format, VolumeBorders border);
    virtual ~VolumeRepresentation();
    virtual void performOperation(DataOperation*) const {};
    // Removes old data and reallocate for new dimension.
    // Needs to be overloaded by child classes.
    virtual void setDimension(uvec3 dimensions) { dimensions_ = dimensions;}

    const VolumeBorders& getBorder() const {return borders_;}
    virtual uvec3 getBorderLLF() const {return borders_.llf;}
    virtual uvec3 getBorderURB() const {return borders_.urb;}
    const uvec3& getDimension() const {return dimensions_;}
    uvec3 getDimensionWithBorder() const { return dimensions_+getBorderLLF()+getBorderURB(); }
    bool hasBorder() const {return borders_.hasBorder;}
protected:
    uvec3 dimensions_;
    VolumeBorders borders_;
};

} // namespace

#endif // IVW_VOLUMEREPRESENTATION_H
