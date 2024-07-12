// Params: Mission result, Text message (optional), Next mission (default=IdleMission)


void run()
{
    State state;
    Result result;
    Setpoint deliveryPosition;

    result = TakeOffWarning(duration_s=0.25).run();

    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed run takeOff warning!", Missions::IdleMission);
    }

    result = TakeOff(height=1.0);
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed run take off, shutting off motors!", LAND??);
    }

    result = AscendTo(height=1.5);
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed to ascend to 1.5 meters, landing again", LAND??);
    }

    result = FlyTo(deliveryPosition);
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed to fly to delivery position, returning home", RTL??);
    }

    result = FindQrCode(timeout_s=60);
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed to find QR code at delivery destination, returning home", RTL??);
    }

    result = DescendForDelivery(drop_height_cm=5);
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed to find descend to delivery height, returning home", RTL??);
    }

    result = DeliverBeer();
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed to deliver beer at destination, returning home", RTL??);
    }

    result = AscendTo(height=1.5);
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed to find descend to delivery height, returning home", RTL??);
    }

    result = RTL();
    if (result == State::Result::FAIL)
    {
        return stop(Mission::Result::FAIL, "Failed with RTL", LAND?);
    }

}