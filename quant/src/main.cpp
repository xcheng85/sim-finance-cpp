#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/schedule.hpp>
#include <iostream>

int main()
{

    using namespace QuantLib;

    auto valueDate = Date(8, January, 2025);
    Settings::instance().evaluationDate() = valueDate;

    Schedule schedule =
        MakeSchedule()
            .from(Date(26, August, 2024))
            .to(Date(26, May, 2026))
            .withFirstDate(Date(26, August, 2024))
            .withFrequency(Semiannual)
            .withCalendar(TARGET())
            .withConvention(Following)
            .backwards();

    for (auto d : schedule)
    {
        std::cout << d << "\n";
    }
    std::cout << std::endl;
}