
local aimKey = getgenv().osu.Aimbot.Keybind
local aimAssistEnabled = false
local currentTarget = nil

-- Create FOV Circle GUI
local function createFOVCircle()
    local player = game.Players.LocalPlayer
    local playerGui = player:FindFirstChild("PlayerGui")

    if not playerGui then return end

    local fovCircle = Instance.new("Frame")
    fovCircle.Name = "FOVCircle"
    fovCircle.Size = UDim2.new(0, getgenv().osu.FieldOfView.Size * 2, 0, getgenv().osu.FieldOfView.Size * 2)
    fovCircle.BackgroundColor3 = getgenv().osu.FieldOfView.Color
    fovCircle.BackgroundTransparency = getgenv().osu.FieldOfView.Transparency
    fovCircle.BorderSizePixel = 0
    fovCircle.AnchorPoint = Vector2.new(0.5, 0.5)

    local uicorner = Instance.new("UICorner")
    uicorner.CornerRadius = UDim.new(0.5, 0)
    uicorner.Parent = fovCircle

    fovCircle.Parent = playerGui
    return fovCircle
end

local fovCircle = createFOVCircle()

local function isWithinFOV(position, size)
    local camera = workspace.CurrentCamera
    local viewportPosition = camera:WorldToViewportPoint(position)
    local mousePosition = game.Players.LocalPlayer:GetMouse()

    local distanceFromCenter = (Vector2.new(viewportPosition.X, viewportPosition.Y) - Vector2.new(mousePosition.X, mousePosition.Y)).magnitude
    return distanceFromCenter <= size
end

local function isTargetAlive(target)
    local humanoid = target.Character:FindFirstChildOfClass("Humanoid")
    return humanoid and humanoid.Health > getgenv().osu.Checks.MinHealth and humanoid.Health > 0
end

local function getNearestTarget()
    local nearestTarget = nil
    local shortestDistance = math.huge

    for _, player in pairs(game.Players:GetPlayers()) do
        if player ~= game.Players.LocalPlayer and player.Character then
            if (getgenv().osu.Checks.DeathCheckEnabled and not isTargetAlive(player)) or
               (getgenv().osu.Checks.HealthCheckEnabled and (player.Character:FindFirstChildOfClass("Humanoid").Health < getgenv().osu.Checks.MinHealth)) then
                continue
            end

            local targetPart = player.Character:FindFirstChild(getgenv().osu.Aimbot.AimPart)
            if targetPart and isWithinFOV(targetPart.Position, getgenv().osu.FieldOfView.Size) then
                local targetPosition = targetPart.Position
                local distance = (targetPosition - game.Players.LocalPlayer.Character.HumanoidRootPart.Position).magnitude

                if distance < shortestDistance then
                    nearestTarget = player
                    shortestDistance = distance
                end
            end
        end
    end

    return nearestTarget
end

local function predictTargetPosition(target)
    if target and target.Character then
        local targetPart = target.Character:FindFirstChild(getgenv().osu.Aimbot.AimPart)
        if targetPart and target.Character:FindFirstChild("Humanoid") then
            local humanoid = target.Character.Humanoid
            local velocity = humanoid.RootPart.Velocity -- Get the velocity of the target
            local predictionTime = 0.1 -- Time to predict
            local predictedPosition = targetPart.Position + (velocity * predictionTime) -- Predict future position
            return predictedPosition
        end
    end
    return nil
end

local function aimAtTarget(target)
    if target and target.Character then
        local predictedPosition = predictTargetPosition(target)
        if predictedPosition then
            local camera = workspace.CurrentCamera
            local aimPosition = camera:WorldToViewportPoint(predictedPosition)

            local mouse = game.Players.LocalPlayer:GetMouse()
            local currentX = mouse.X
            local currentY = mouse.Y

            -- Calculate delta
            local deltaX = (aimPosition.X - currentX) * getgenv().osu.Aimbot.Smoothness
            local deltaY = (aimPosition.Y - currentY) * getgenv().osu.Aimbot.Smoothness

            -- Move mouse smoothly
            mousemoverel(deltaX, deltaY)
        end
    end
end

-- Toggle aimbot
game:GetService("UserInputService").InputBegan:Connect(function(input, gameProcessedEvent)
    if input.KeyCode.Name == aimKey and not gameProcessedEvent then  -- Check the name of the key
        aimAssistEnabled = not aimAssistEnabled
        if aimAssistEnabled then
            currentTarget = getNearestTarget() -- Set the target when enabling
        else
            currentTarget = nil -- Clear the target when disabling
        end
    end
end)

-- Aim loop
game:GetService("RunService").RenderStepped:Connect(function()
    if aimAssistEnabled then
        if currentTarget then
            aimAtTarget(currentTarget) -- Keep aiming at the target
        else
            currentTarget = getNearestTarget() -- Get a new target if there isn't one
        end
    end
end)
