#pragma once

#include "io.hpp"


class ErrorIndicator
{
    public:
        ErrorIndicator(GPIO &out);
        void indicate_status(uint32_t status);

    private:
        GPIO &out;
};
