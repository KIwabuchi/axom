// wrapExClass1.cpp
// This is generated code, do not edit
// blah blah
// yada yada
//
// wrapExClass1.cpp
#define EXAMPLE_WRAPPER_IMPL
#include "wrapExClass1.h"
#include "ExClass1.hpp"

extern "C" {
namespace example {
namespace nested {

AA_exclass1 * AA_exclass1_new(const char * name)
{
ExClass1 *selfobj = new ExClass1(name);
// splicer begin class.ExClass1.method.new
return (AA_exclass1 *) selfobj;
// splicer end class.ExClass1.method.new
}

AA_exclass1 * AA_exclass1_new_bufferify(const char * name, int Lname)
{
ExClass1 *selfobj = new ExClass1(std::string(name, Lname));
// splicer begin class.ExClass1.method.new_bufferify
return (AA_exclass1 *) selfobj;
// splicer end class.ExClass1.method.new_bufferify
}

void AA_exclass1_delete(AA_exclass1 * self)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.delete
delete selfobj;
// splicer end class.ExClass1.method.delete
}

int AA_exclass1_increment_count(AA_exclass1 * self, int incr)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.increment_count
int rv = selfobj->incrementCount(incr);
return rv;
// splicer end class.ExClass1.method.increment_count
}

const char * AA_exclass1_get_name(const AA_exclass1 * self)
{
const ExClass1 *selfobj = static_cast<const ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_name
const std::string & rv = selfobj->getName();
if (! isNameValid(rv)) {
    return NULL;
}

return rv.c_str();
// splicer end class.ExClass1.method.get_name
}

int AA_exclass1_get_name_length(AA_exclass1 * self)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_name_length
return selfobj->getName().length();
// splicer end class.ExClass1.method.get_name_length
}

const char * AA_exclass1_get_name_error_check(const AA_exclass1 * self)
{
const ExClass1 *selfobj = static_cast<const ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_name_error_check
const std::string & rv = selfobj->getNameErrorCheck();
return rv.c_str();
// splicer end class.ExClass1.method.get_name_error_check
}

const char * AA_exclass1_get_name_arg(const AA_exclass1 * self)
{
const ExClass1 *selfobj = static_cast<const ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_name_arg
const std::string & rv = selfobj->getNameArg();
return rv.c_str();
// splicer end class.ExClass1.method.get_name_arg
}

AA_exclass2 * AA_exclass1_get_root(AA_exclass1 * self)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_root
ExClass2 * rv = selfobj->getRoot();
return rv;
// splicer end class.ExClass1.method.get_root
}

int AA_exclass1_get_value_from_int(AA_exclass1 * self, int value)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_value_from_int
int rv = selfobj->getValue(value);
return rv;
// splicer end class.ExClass1.method.get_value_from_int
}

long AA_exclass1_get_value_1(AA_exclass1 * self, long value)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_value_1
long rv = selfobj->getValue(value);
return rv;
// splicer end class.ExClass1.method.get_value_1
}

void * AA_exclass1_get_addr(AA_exclass1 * self)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.get_addr
void * rv = selfobj->getAddr();
return rv;
// splicer end class.ExClass1.method.get_addr
}

bool AA_exclass1_has_addr(AA_exclass1 * self, bool in)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.has_addr
bool rv = selfobj->hasAddr(in);
return rv;
// splicer end class.ExClass1.method.has_addr
}

void AA_exclass1_splicer_special(AA_exclass1 * self)
{
ExClass1 *selfobj = static_cast<ExClass1 *>(self);
// splicer begin class.ExClass1.method.splicer_special
//   splicer for SplicerSpecial
// splicer end class.ExClass1.method.splicer_special
}

// splicer begin class.ExClass1.additional_functions
// splicer end class.ExClass1.additional_functions

}  // namespace example
}  // namespace nested
}  // extern "C"
