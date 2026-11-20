INCLUDE Irvine32.inc
INCLUDE Core.inc
INCLUDE UI.inc

.code

; ============================================================
; HandleEntry
; ============================================================

HandleEntry PROC
    call VehicleEntry

    cmp eax, 1
    jne EntryHandled

    inc TotalEntered

    cmp InputVehicleType, CAR
    jne CountBike

    inc CarsEntered ;increment car count
    jmp EntryHandled

    CountBike:
        ;increments bikes count
        inc BikesEntered

    EntryHandled:
        ret

HandleEntry ENDP

HandleExit PROC USES ebx ecx edx
    mov ebx, OccupancyMask ;occupancy before exiting

    call VehicleExit

    cmp eax, 1
    jne ExitHandled ;if exit not handled

    inc TotalExited

    ; Find the bit cleared by VehicleExit
    mov eax, ebx
    xor eax, OccupancyMask ;the slot that wanted to be exited is not cleared
    ;here eax is old occupancy mask and new occupancy mask is inside occupancy mask

    mov ecx, 0

    ;for parking fees
    FindBit:
        cmp ecx, SLOTS
        jae ExitHandled

        shr eax, 1
        jc BitFound

        inc ecx
        jmp FindBit

    BitFound:
        mov edx, ParkingFee
        mov al, VehicleTypes[ecx]

        cmp al, CAR
        jne BikeExited

        add CarRevenue, edx
        jmp ExitHandled

    BikeExited:
        add BikeRevenue, edx

    ExitHandled:
        ret

HandleExit ENDP

main PROC

    MainLoop:
        call Clrscr

        ; Main Menu
        call DisplayHeader
        call DisplayMenu

        ; Read numeric menu selection
        call ReadDec

        cmp eax, 1
        je Choice1

        cmp eax, 2
        je Choice2

        cmp eax, 3
        je Choice3

        cmp eax, 4
        je Choice4

        cmp eax, 5
        je Choice5

        cmp eax, 6
        je Choice6

        cmp eax, 7
        je Choice7

        cmp eax, 8
        je Choice8

        jmp BadChoice

        ;vehicle entry
        Choice1:
            call Clrscr
            mov eax, lightCyan
            call SetTextColor
            call DisplayHeader
            call HandleEntry
            jmp PauseScreen


        ;vehicle exit
        Choice2:
            call Clrscr
            mov eax, lightCyan
            call SetTextColor
            call DisplayHeader
            call HandleExit
            jmp PauseScreen

        ;parking map (occupancy mask)
        Choice3:
            call Clrscr
            call DisplayHeader
            call DisplayParkingMap
            jmp PauseScreen

        ; Searching for a vehicle
        Choice4:
            call Clrscr
            mov eax, lightCyan
            call SetTextColor
            call DisplayHeader
            call SearchVehicle
            jmp PauseScreen

        ;statistics
        Choice5:
            call Clrscr
            call DisplayHeader
            call DisplayOccupancy
            call Crlf
            call DisplayStatistics
            jmp PauseScreen

        ;REVENUE REPORT
        Choice6:
            call Clrscr
            call DisplayHeader
            call DisplayRevenue
            jmp PauseScreen 

        ;Traffic Analysis Report
        Choice7:
            call Clrscr
            call DisplayHeader
            call DisplayTrafficAnalysis
            jmp PauseScreen

        ;EXIT
        Choice8:
            call Clrscr
            call DisplayHeader
            call DisplayFooter
            exit

        ; INVALID MENU CHOICE
        BadChoice:
            mov eax, lightRed
            call SetTextColor
            call Crlf

            mov edx, OFFSET uiInvalidChoice
            call WriteString
            call Crlf

            mov eax, white
            call SetTextColor

            jmp PauseScreen


        ; PAUSE BEFORE RETURNING TO MENU
        PauseScreen:
            call Crlf

            mov eax, yellow
            call SetTextColor

            call WaitMsg

            mov eax, white
            call SetTextColor

        jmp MainLoop


main ENDP

END main
