// Copyright (c) Vorne Industries
//
// [FUTUREHACK] Quick and dirty way to get a PRU loaded with alphanumeric display code.

#include <iostream>
#include <stdexcept>
#include <string>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// PruMapper_Interface

class PruMapper_Interface
{
public:
    virtual ~PruMapper_Interface() {}

    /**
     * Get a pointer to the PRU memory.
     */
    virtual void * get() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PruMapper_Real

class PruMapper_Real : public PruMapper_Interface
{
public:
    /**
     * Class constructor.
     *
     * @throws std::runtime_error Encountered a runtime error.
     */
    PruMapper_Real()
    {
        // Allocate and initialize memory
        if (prussdrv_init())
        {
            throw std::runtime_error("pru-driver: Call to prussdrv_init() failed");
        }

        // Open PRU Interrupt
        if (prussdrv_open(get_host_interrupt()))
        {
            std::string message;

            message
                .append("pru-driver: Call to prussdrv_open(")
                .append(std::to_string(get_host_interrupt()))
                .append(") failed")
                ;

            throw std::runtime_error(message);
        }

        // Map shared PRUs memory
        if (prussdrv_map_prumem(get_pru_ram_id(), &m_val))
        {
            std::string message;

            message
                .append("pru-driver: Call to prussdrv_map_prumem(")
                .append(std::to_string(get_pru_ram_id()))
                .append(") failed.")
                ;

            throw std::runtime_error(message);
        }

        // Clear the display memory to avoid displaying noise at power-up.
        std::memset(m_val, 0, DISPLAY_SIZE);

        // Keep track of the file descriptor following a successful call to prussdrv_open().
        m_fd = prussdrv_pru_event_fd(get_host_interrupt());

        {
            tpruss_intc_initdata const pruss_intc_initdata = PRUSS_INTC_INITDATA;

            if (prussdrv_pruintc_init(&pruss_intc_initdata))
            {
                throw std::runtime_error("pru-driver: Call to prussdrv_pruintc_init() failed");
            }
        }

        if (prussdrv_load_datafile(get_pru_num(), get_data_file_name()))
        {
            std::string message;

            message
                .append("pru-driver: Call to prussdrv_load_datafile(")
                .append(std::to_string(get_pru_num()))
                .append(", \"")
                .append(get_data_file_name())
                .append("\") failed")
                ;

            throw std::runtime_error(message);
        }

        // Load/exec the bin in PRU
        if (prussdrv_exec_program(get_pru_num(), get_text_file_name()))
        {
            std::string message;

            message
                .append("pru-driver: Call to prussdrv_exec_program(")
                .append(std::to_string(get_pru_num()))
                .append(", \"")
                .append(get_text_file_name())
                .append("\") failed")
                ;

            throw std::runtime_error(message);
        }

        std::cout << "pru-driver: PRU " << get_pru_num() << " enabled." << std::endl;
    }

    ~PruMapper_Real() override
    {
        // Don't disable the PRU (don't call prussdrv_pru_disable()). Disabling the Scoreboard PRU
        // may cause LED's to be left enabled which could lead to premature pixel burnout. There's
        // no harm in leaving the Scoreboard up and running.
        //
        std::cout
            << "pru-driver: Intentionally leaving PRU "
            << get_pru_num()
            << " enabled."
            << std::endl
            ;

        if (-1 != m_fd)
        {
            if (-1 == close(m_fd))
            {
                std::cerr << "pru-driver: Call to close(" << m_fd << ") failed." << std::endl;
            }
        }
    }

    void * get() override
    {
        return m_val;
    }

private:

    static unsigned int get_host_interrupt()
    {
        return PRU_EVTOUT_1;
    }

    static int get_pru_num()
    {
        return 1;
    }

    static char const * get_data_file_name()
    {
        return "/lib/firmware/pru-display/data.bin";
    }

    static char const * get_text_file_name()
    {
        return "/lib/firmware/pru-display/text.bin";
    }

    static unsigned int get_pru_ram_id()
    {
        return PRUSS0_SHARED_DATARAM;
    }

    int m_fd { -1 };
    void * m_val { nullptr };

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PruMapper_Virtual

class PruMapper_Virtual : public PruMapper_Interface
{
public:

    void * get() override
    {
        return m_val;
    }

private:

    uint8_t m_val[ DISPLAY_SIZE + /*display_control_width*/ 1 ] {};

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PruMapper

class PruMapper : public
    #if defined(RUNNING_ON_DEVICE) && !defined(USE_HD_SCOREBOARD)
    PruMapper_Real
    #else
    PruMapper_Virtual
    #endif
{
};

