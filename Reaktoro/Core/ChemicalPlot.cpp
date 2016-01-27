// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2015 Allan Leal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "ChemicalPlot.hpp"

// C++ includes
#include <cstdio>
#include <fstream>
#include <iomanip>

// Boost includes
#include <boost/format.hpp>

// Reaktoro includes
#include <Reaktoro/Common/Units.hpp>
#include <Reaktoro/Common/StringUtils.hpp>
#include <Reaktoro/Core/ChemicalQuantity.hpp>
#include <Reaktoro/Core/ChemicalState.hpp>
#include <Reaktoro/Core/ChemicalSystem.hpp>
#include <Reaktoro/Core/ReactionSystem.hpp>

// Ensure appropriate popen or pclose calls when compiling with MSVC
#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#endif

namespace Reaktoro {
namespace {

const std::string gnuplot_preamble = R"(
# Change the font
set termoption enhanced
set termoption font "Verdana,14"

# Set a smaller font for the legend
set key font ",12"

# Set a thick border
set border linewidth 3

# The line styles
set style line 1 lt 2 lw 3 lc rgb '#0072bd' # blue
set style line 2 lt 2 lw 3 lc rgb '#d95319' # orange
set style line 3 lt 2 lw 3 lc rgb '#edb120' # yellow
set style line 4 lt 2 lw 3 lc rgb '#7e2f8e' # purple
set style line 5 lt 2 lw 3 lc rgb '#77ac30' # green
set style line 6 lt 2 lw 3 lc rgb '#4dbeee' # light-blue
set style line 7 lt 2 lw 3 lc rgb '#a2142f' # red
)";

} // namespace

struct ChemicalPlot::Impl
{
    /// The chemical system instance
    ChemicalSystem system;

    /// The reaction system instance
    ReactionSystem reactions;

    /// The chemical quantity instance
    ChemicalQuantity quantity;

    /// The name of the plot.
    std::string name;

    /// The quantity that spans the x-axis
    std::string x = "t";

    /// The quantities to be plotted along the y-axis
	std::vector<std::string> y = std::vector<std::string>({"t"}); // need explicit initialization list to compile in MSVC 2013

    /// The names of each curve given by member `y`.
    std::vector<std::string> legend;

    /// The boolean flag that indicates if the legend should be hidden
    bool nolegend = false;

    /// The Gnuplot commands used to configure the plot.
    std::string config;

    /// The frequency in which the plot is refreshed per second.
    unsigned frequency = 30;

    /// The name of the data file.
    std::string dataname;

    /// The name of the gnuplot script file.
    std::string plotname;

    /// The name of the file that is open to signal Gnuplot to start rereading.
    std::string endname;

    /// The output stream of the data file.
    std::ofstream datafile;

    /// The output stream of the gnuplot script file.
    std::ofstream plotfile;

    /// The file that is open to signal Gnuplot to start rereading.
    std::ofstream endfile;

    /// The pointer to the pipe connecting to Gnuplot
    FILE* pipe = nullptr;

    /// The counter of ChemicalPlot instances
    static unsigned counter;

    /// The ID of this ChemicalPlot instance (by order of creation)
    unsigned id;

    Impl()
    {
        id = counter++;
    }

    Impl(const ChemicalSystem& system)
    : system(system), quantity(system)
    {
        id = counter++;
    }

    Impl(const ReactionSystem& reactions)
    : system(reactions.system()), reactions(reactions), quantity(reactions)
    {
        id = counter++;
    }

    ~Impl()
    {
        close();
    }

    auto open() -> void
    {
        // Ensure the plot is closed
        close();

        // Make sure name is not empty
        if(name.empty())
            name = "plot" + std::to_string(id);

        // Make sure legend is not empty
        if(legend.empty() && !nolegend)
            legend = y;

        // Initialize the names of the data and gnuplot script files
        dataname = name + ".dat";
        plotname = name + ".plt";
        endname  = name + ".end";

        // Open the data and gnuplot script files
        datafile.open(dataname);
        plotfile.open(plotname);

        // Output the name of each quantity in the data file
        datafile << std::left << std::setw(20) << x;
        for(auto yi : y)
            datafile << std::left << std::setw(20) << yi;
        datafile << std::endl;

        // Initialize the Gnuplot script file with the default preamble
        plotfile << gnuplot_preamble;

        // Apply the custom configuration
        plotfile << config;

        // Define a Gnuplot variable that is a list of titles for the curves
        plotfile << "titles = \""; // e.g., titles = "'legend1' 'legend2' 'legend3'"
        for(unsigned i = 0; i < legend.size(); ++i)
            plotfile << (i == 0 ? "" : " ") << "'" << legend[i] << "'";
        plotfile << "\"\n" << std::endl;

        // Define the formatted string that represents the plot part of the Gnuplot script
        std::string script = R"xyz(
previous = current
current = system('%1% %2%')
finished = system('%3%')
pause %4%
if(current ne previous && previous ne '') \
    plot for [i=2:%5%] '%2%' using 1:i with lines ls i-1 title word(titles, i-1)
if(finished == 0) reread)xyz";

        // On Windows, use the `dir` command on the data file to check its state.
        // On any other OS, use the `ls -l` command instead.
#if _WIN32
        std::string file_status_cmd = "dir";
        std::string file_exists_cmd = "if exist " + endname + " (echo 1) else (echo 0)";
#else
        std::string file_status_cmd = "ls -l";
        std::string file_exists_cmd = "[ ! -e " + endname + " ]; echo $?";
#endif
        // Define auxiliary variables for the plot
        auto imax = 1 + y.size();
        auto wait = 1.0/frequency;

        // Finalize the Gnuplot script
        plotfile << boost::format(script) % file_status_cmd % dataname % file_exists_cmd % wait % imax;

        // Flush the plot file to ensure its correct state before the plot starts
        plotfile.flush();
    }

    auto close() -> void
    {
        if(pipe != nullptr)
        {
            // Create the file that signals Gnuplot to stop rereading the input script
            endfile.open(endname);

            // Close the pipe
            pclose(pipe);

            // Close the previously created file
            endfile.close();

            // Delete the end file
            std::remove(endname.c_str());

            // Set pipe to nullptr
            pipe = nullptr;
        }
    }

    auto update(const ChemicalState& state, double t) -> void
    {
        // Output the current chemical state to the data file.
        quantity.update(state, t);
        datafile << std::left << std::setw(20) << quantity.value(x);
        for(auto word : y)
            datafile << std::left << std::setw(20) << quantity.value(word);
        datafile << std::endl;

        // Open the Gnuplot plot after the first data has been output to the data file.
        // This ensures that Gnuplot opens the plot without errors/warnings.
        if(pipe == nullptr)
        {
            auto command = ("gnuplot -persist -e \"current=''\" " + plotname).c_str();
            pipe = popen(command, "w");
        }
    }
};

// Initialize the counter of ChemicalPlot instances
unsigned ChemicalPlot::Impl::counter = 0;

ChemicalPlot::ChemicalPlot()
: pimpl(new Impl())
{}

ChemicalPlot::ChemicalPlot(const ChemicalSystem& system)
: pimpl(new Impl(system))
{}

ChemicalPlot::ChemicalPlot(const ReactionSystem& reactions)
: pimpl(new Impl(reactions))
{}

ChemicalPlot::~ChemicalPlot()
{}

auto ChemicalPlot::name(std::string name) -> void
{
    pimpl->name = name;
}

auto ChemicalPlot::xdata(std::string x) -> void
{
    pimpl->x = x;
}

auto ChemicalPlot::ydata(std::vector<std::string> y) -> void
{
    pimpl->y = y;
}

auto ChemicalPlot::ydata(std::string y) -> void
{
    pimpl->y = splitrim(y, ";\n");
}

auto ChemicalPlot::xlabel(std::string str) -> void
{
    *this << "set xlabel '" + str + "'";
}

auto ChemicalPlot::ylabel(std::string str) -> void
{
    *this << "set ylabel '" + str + "' offset 2";
}

auto ChemicalPlot::xtics(std::string str) -> void
{
    *this << "set xtics (" + str + ")";
}

auto ChemicalPlot::ytics(std::string str) -> void
{
    *this << "set ytics (" + str + ")";
}

auto ChemicalPlot::xformat(std::string str) -> void
{
    *this << "set format x '" + str + "'";
}

auto ChemicalPlot::yformat(std::string str) -> void
{
    *this << "set format y '" + str + "'";
}

auto ChemicalPlot::xlogscale(int base) -> void
{
    *this << "set logscale x " + std::to_string(base);
}

auto ChemicalPlot::ylogscale(int base) -> void
{
    *this << "set logscale y " + std::to_string(base);
}

auto ChemicalPlot::legend(std::vector<std::string> legend) -> void
{
    pimpl->nolegend = false;
    pimpl->legend = legend;
}

auto ChemicalPlot::legend(std::string legend) -> void
{
    pimpl->nolegend = false;
    pimpl->legend = splitrim(legend, ";\n");
}

auto ChemicalPlot::nolegend() -> void
{
    pimpl->nolegend = true;
}

auto ChemicalPlot::frequency(unsigned frequency) -> void
{
    pimpl->frequency = frequency;
}

auto ChemicalPlot::operator<<(std::string command) -> ChemicalPlot&
{
    pimpl->config.append(command + "\n");
    return *this;
}

auto ChemicalPlot::operator<<(std::stringstream command) -> ChemicalPlot&
{
    pimpl->config.append(command.str());
    return *this;
}

auto ChemicalPlot::open() -> void
{
    pimpl->open();
}

auto ChemicalPlot::update(const ChemicalState& state, double t) -> void
{
    pimpl->update(state, t);
}

auto ChemicalPlot::operator==(const ChemicalPlot& other) -> bool
{
    return pimpl == other.pimpl;
}

} // namespace Reaktoro
