/***********************************************************************************************************************
 *   LibDegorasSLR (Degoras Project SLR Library).                                                                      *
 *                                                                                                                     *
 *   A modern and efficient C++ base library for Satellite Laser Ranging (SLR) software and real-time hardware         *
 *   related developments. Developed as a free software under the context of Degoras Project for the Spanish Navy      *
 *   Observatory SLR station (SFEL) in San Fernando and, of course, for any other station that wants to use it!        *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *
 *                                                                                                                     *
 *   This file is part of LibDegorasSLR.                                                                               *
 *                                                                                                                     *
 *   Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license   *
 *   as soon they will be approved by the European Commission (IDABC).                                                 *
 *                                                                                                                     *
 *   This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as    *
 *   published by the IDABC, either Version 1.2 or, at your option, any later version.                                 *
 *                                                                                                                     *
 *   This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in *
 *   writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the  *
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific   *
 *   language governing permissions and limitations and more details.                                                  *
 *                                                                                                                     *
 *   You should use this project in compliance with the EUPL license. You should have received a copy of the license   *
 *   along with this project. If not, see the license at < https://eupl.eu/ >.                                         *
 **********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
// =====================================================================================================================


// LIBDEGORASSLR INCLUDES
// =====================================================================================================================
#include <LibDegorasSLR/UtilitiesSLR/tracking_slr.h>
#include <LibDegorasSLR/Timing/types/time_types.h>
// =====================================================================================================================

// NAMESPACES
// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------

int main()
{

    // LibDegorasSLR types used in example.
    using dpslr::ilrs::cpf::CPF;
    using dpslr::geo::types::GeocentricPoint;
    using dpslr::geo::types::GeodeticPoint;
    using dpslr::utils::PredictorSLR;
    using dpslr::utils::TrackingSLR;
    using dpslr::timing::MJDate;
    using dpslr::timing::SoD;
    using dpslr::math::units::Angle;

    // Configure the CPF folder and example file.
    std::string cpf_dir("../resources/CPF/");

    // CPF name and start and end date for trackings.

    // Lares. Sun at beginning
    std::string cpf_name("38077_cpf_240128_02901.sgf");
    MJDate mjd_start = 60340;
    SoD sod_start = 56726;
    MJDate mjd_end = 60340;
    SoD sod_end = 57756;

    // Jason 3. Sun in the middle.
    // std::string cpf_name("41240_cpf_240128_02801.hts");
    // timing::MJDate mjd_start = 60340;
    // timing::SoD sod_start = 42140;
    // timing::MJDate mjd_end = 60340;
    // timing::SoD sod_end = 43150;

    // Explorer27. Sun in the end.
    // std::string cpf_name("1328_cpf_240128_02901.sgf");
    // timing::MJDate mjd_start = 60340;
    // timing::SoD sod_start = 30687;
    // timing::MJDate mjd_end = 60340;
    // timing::SoD sod_end = 31467;


    // SFEL station geodetic coordinates.
    long double latitude = 36.46525556L, longitude = 353.79469440L, alt = 98.177L;

    // SFEL station geocentric coordinates
    long double x = 5105473.885L, y = -555110.526L, z = 3769892.958L;


    // Store the local coordinates.
    GeocentricPoint<long double> stat_geocentric(x,y,z);
    GeodeticPoint<long double> stat_geodetic(latitude, longitude, alt, Angle<long double>::Unit::DEGREES);

    // Open the CPF file.
    CPF cpf(cpf_dir + cpf_name, dpslr::ilrs::cpf::CPF::OpenOptionEnum::ALL_DATA);

    // Configure the SLR predictor.
    PredictorSLR predictor(cpf, stat_geodetic, stat_geocentric);
    predictor.setPredictionMode(PredictorSLR::PredictionMode::INSTANT_VECTOR);

    if (!predictor.isReady())
    {
        std::cerr << "The predictor has no data valid to predict." << std::endl;
        return -1;
    }

    // Configure the SLR tracking passing the predictor, the start and end dates and minimum elevation (optional).
    TrackingSLR tracking(std::move(predictor), mjd_start, sod_start, mjd_end, sod_end, 8);

    if (!tracking.isValid())
    {
        std::cerr << "There is no valid tracking." << std::endl;
        return -1;
    }

    // Check for sun overlapping in tracking. If there is sun overlapping at start or end, the affected date is changed
    // so the tracking will start or end after/before the sun security sector.
    if (tracking.isSunOverlapping())
    {
        std::cout << "There is sun overlapping" << std::endl;

        if (tracking.isSunAtStart())
        {
            std::cout << "Sun overlapping at the beginning" << std::endl;
            // Get the new tracking start date
            tracking.getTrackingStart(mjd_start, sod_start);
        }

        if (tracking.isSunAtEnd())
        {
            std::cout << "Sun overlapping at the end" << std::endl;
            // Get the new tracking end date.
            tracking.getTrackingEnd(mjd_end, sod_end);
        }
    }

    // Now, we have the tracking configured, so we can ask the tracking to predict any position within the valid
    // tracking time window (determined by tracking start and tracking end). For the example, we will ask
    // predictions from start to end with a step of 0.5 s.
    MJDate mjd = mjd_start;
    SoD sod = sod_start;
    TrackingSLR::TrackingPredictions results;

    while (mjd < mjd_end || sod < sod_end)
    {

        // Store the resulting prediction
        results.push_back({});
        auto status = tracking.predict(mjd, sod, results.back());

        if (status == TrackingSLR::INSIDE_SUN)
        {
            // In this case the position predicted is valid, but it is going through a sun security sector.
            // This case is only possible if sun avoid algorithm is disabled.
            // BEWARE. If the mount points directly to this position it could be dangerous.
        }
        else if (status == TrackingSLR::OUTSIDE_SUN)
        {
            // In this case the position predicted is valid and it is going outside a sun security sector. This is the
            // normal case.
        }
        else if (status == TrackingSLR::AVOIDING_SUN)
        {
            // In this case the position predicted is valid and it is going through an alternative way to avoid a sun
            // security sector. While the tracking returns this status, the tracking_position member in result
            // represents the position used to avoid the sun (the secure position), while prediction_result contains
            // the true position of the object (not secure).

        }
        else
        {
            std::cout << "Error at getting position " << status;
            return -1;
        }

        // Advance to next position
        sod += 0.5L;
        if (sod > 86400.L)
        {
            sod -= 86400.L;
            mjd++;
        }
    }

    // We will store the positions in a file. This could be used for graphical representation.
    // We will also store the sun position at each tracking position.
    std::ofstream file_pos("./tracking.txt", std::ios_base::out);
    std::ofstream file_pos_sun("./pos_sun.txt", std::ios_base::out);

    for (const auto &prediction : results)
    {
        file_pos << prediction.tracking_position->az << ", " << prediction.tracking_position->el << std::endl;
        file_pos_sun << prediction.sun_pos->azimuth << ", " << prediction.sun_pos->elevation << std::endl;
    }

    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================
