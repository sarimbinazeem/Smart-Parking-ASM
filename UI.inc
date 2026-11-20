
.data
DIV_WIDTH  = 56
MENU_COUNT = 8

; Extra statistics
TotalEntered  DWORD 0
TotalExited   DWORD 0
CarsEntered   DWORD 0
BikesEntered  DWORD 0
CarRevenue    DWORD 0
BikeRevenue   DWORD 0

; Header
uiTitle1 BYTE "              SMARTPARK-ASM", 0
uiTitle2 BYTE "       CAMPUS PARKING MANAGEMENT SYSTEM", 0
uiSubtitle BYTE "          SMART PARKING CONTROL TERMINAL", 0

; Menu
uiMenuTitle BYTE "                    MAIN MENU", 0
uiMenu1 BYTE "    [1]  Vehicle Entry", 0
uiMenu2 BYTE "    [2]  Vehicle Exit", 0
uiMenu3 BYTE "    [3]  Parking Map", 0
uiMenu4 BYTE "    [4]  Search Vehicle", 0
uiMenu5 BYTE "    [5]  Statistics", 0
uiMenu6 BYTE "    [6]  Revenue Report", 0
uiMenu7 BYTE "    [7]  Traffic Analysis", 0
uiMenu8 BYTE "    [8]  Exit", 0
MenuItems DWORD OFFSET uiMenu1, OFFSET uiMenu2
          DWORD OFFSET uiMenu3, OFFSET uiMenu4
          DWORD OFFSET uiMenu5, OFFSET uiMenu6
          DWORD OFFSET uiMenu7, OFFSET uiMenu8

uiPrompt        BYTE "    Select an option (1-8): ", 0
uiInvalidChoice BYTE "Invalid choice! Select a number from 1 to 8.", 0
uiBye           BYTE "Thank you for using SmartPark-ASM!", 0

; Parking map
uiMapTitle BYTE "                    PARKING MAP", 0
uiLegend   BYTE "  [X] = OCCUPIED       [ ] = AVAILABLE", 0
uiGap      BYTE "    ", 0

; Occupancy
uiOccTitle    BYTE "                 OCCUPANCY OVERVIEW", 0
uiTotalSlots  BYTE "  Total Slots       : ", 0
uiOccupied    BYTE "  Occupied Slots    : ", 0
uiAvailable   BYTE "  Available Slots   : ", 0
uiOccPercent  BYTE "  Occupancy         : ", 0
uiPercentSign BYTE "%", 0

; Statistics
uiStatTitle  BYTE "                 PARKING STATISTICS", 0
uiEntered    BYTE "  Total Vehicles Entered : ", 0
uiExited     BYTE "  Total Vehicles Exited  : ", 0
uiCars       BYTE "  Cars Entered           : ", 0
uiBikes      BYTE "  Bikes Entered          : ", 0
uiCurrent    BYTE "  Currently Parked       : ", 0
uiCurRevenue BYTE "  Total Revenue (PKR)    : ", 0

; Revenue
uiRevTitle BYTE "                   REVENUE REPORT", 0
uiRevTotal BYTE "  Total Revenue : PKR ", 0
uiRevCars  BYTE "  Cars Revenue  : PKR ", 0
uiRevBikes BYTE "  Bikes Revenue : PKR ", 0

; Traffic
uiTrafTitle BYTE "                   TRAFFIC ANALYSIS", 0

pName0 BYTE "  07:00 - 09:00", 0
pName1 BYTE "  09:00 - 11:00", 0
pName2 BYTE "  11:00 - 13:00", 0
pName3 BYTE "  13:00 - 15:00", 0
pName4 BYTE "  15:00 - 17:00", 0

PeriodNames DWORD OFFSET pName0, OFFSET pName1
            DWORD OFFSET pName2, OFFSET pName3
            DWORD OFFSET pName4

uiColon     BYTE " : ", 0
uiVehWord   BYTE " vehicles", 0
uiPeak      BYTE "  PEAK TRAFFIC PERIOD:", 0
uiNoTraffic BYTE "  No traffic recorded in the monitored periods yet.", 0

; General
uiPressKey BYTE "Press any key to return to the main menu...", 0


.code

; DisplayDivider - the decoration
DisplayDivider PROC USES eax ecx

    mov eax, lightBlue
    call SetTextColor

    mov ecx, DIV_WIDTH
    mov al, '='

    DrawDivider:
        call WriteChar
        loop DrawDivider

     call Crlf

     mov eax, white
     call SetTextColor

     ret

DisplayDivider ENDP

;Titles and subtitles
DisplayHeader PROC USES eax edx
    call DisplayDivider

    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiTitle1
    call WriteString
    call Crlf

    mov eax, white
    call SetTextColor

    mov edx, OFFSET uiTitle2
    call WriteString
    call Crlf

    mov eax, yellow
    call SetTextColor

    mov edx, OFFSET uiSubtitle
    call WriteString
    call Crlf

    call DisplayDivider
    call Crlf

    ret

DisplayHeader ENDP


; DisplayMenu

DisplayMenu PROC USES eax ecx edx esi

    mov eax, yellow
    call SetTextColor

    mov edx, OFFSET uiMenuTitle
    call WriteString
    call Crlf
    call Crlf

    mov esi, OFFSET MenuItems
    mov ecx, MENU_COUNT

    ShowItem:

        mov eax, lightGreen
        call SetTextColor

        mov edx, [esi]
        call WriteString
        call Crlf

        add esi, TYPE MenuItems
        loop ShowItem

    call Crlf

    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiPrompt
    call WriteString

    mov eax, white
    call SetTextColor

    ret

DisplayMenu ENDP


; DisplayFooter

DisplayFooter PROC USES eax edx

    call DisplayDivider
    call Crlf

    mov eax, lightGreen
    call SetTextColor

    mov edx, OFFSET uiBye
    call WriteString
    call Crlf

    call DisplayDivider

    mov eax, white
    call SetTextColor

    ret

DisplayFooter ENDP

ShowValueLine PROC
    ; ShowValueLine
    ; Input:
    ;   EDX = label address
    ;   EAX = number

    push eax

    mov eax, lightCyan
    call SetTextColor

    call WriteString

    mov eax, white
    call SetTextColor

    pop eax
    call WriteDec
    call Crlf

    ret

ShowValueLine ENDP

CountOccupiedSlots PROC USES ebx ecx
    ; CountOccupiedSlots
    ; Returns EAX = number of occupied slots

    mov ebx, OccupancyMask
    mov eax, 0
    mov ecx, SLOTS

    CountLoop:
        ;we are coutning on the basis of carry flag
        shr ebx, 1
        jnc BitIsZero

        inc eax ;count slots

    BitIsZero:
        loop CountLoop

    ret

CountOccupiedSlots ENDP


; Display the occupancy mask map

DisplayParkingMap PROC USES eax edx esi

    ;display =======
    call DisplayDivider

    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiMapTitle
    call WriteString
    call Crlf
    call Crlf

    mov esi, 0 ;slot index

    MapLoop:

        cmp esi, SLOTS
        jge MapDone  ;if all 32 slots traversed

        ; Display slot number
        mov eax, lightCyan
        call SetTextColor

        mov eax, esi
        inc eax  ;slots in 1- 32 format 

        ;for numbers like 01 02 03...
        cmp eax, 10
        jae NoPadding

        ;this is the 0 of 01 02 03
        push eax
        mov al, '0'
        call WriteChar
        pop eax

        NoPadding:
            ;display number
            call WriteDec
            
            mov al, '['
            call WriteChar

            ; Check occupancy
            mov eax, esi
            call CheckSlot

            cmp eax, 1
            je SlotTaken

            ; Available slot
            mov eax, lightGreen
            call SetTextColor

            mov al, ' '
            call WriteChar

            jmp CloseBracket

        SlotTaken:
            ;taken slot is red with X
            mov eax, lightRed
            call SetTextColor

            mov al, 'X'
            call WriteChar

        CloseBracket:

            mov eax, white
            call SetTextColor

            mov al, ']'
            call WriteChar

            mov edx, OFFSET uiGap
            call WriteString

            ; New line after every four slots
            mov eax, esi
            inc eax  ;converted into 1-32 format
            and eax, 3  ;checked if there are 4 slots in row 
            jnz NextSlot

            call Crlf

        NextSlot:
            inc esi
            jmp MapLoop

    MapDone:

        call Crlf

        mov eax, yellow
        call SetTextColor

        mov edx, OFFSET uiLegend  ; showing [] means available and [x] means occupied
        call WriteString
        call Crlf

        mov eax, white
        call SetTextColor

        call DisplayDivider

    ret

DisplayParkingMap ENDP


;Show occupancy statistics (total slots, occupied slot, available slot, occupancy percentage)
DisplayOccupancy PROC USES eax ebx edx esi

    call DisplayDivider

    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiOccTitle
    call WriteString
    call Crlf
    call Crlf

    call CountOccupiedSlots
    mov esi, eax

    mov edx, OFFSET uiTotalSlots
    mov eax, SLOTS
    call ShowValueLine  ;prints string and its value

    mov edx, OFFSET uiOccupied
    mov eax, esi
    call ShowValueLine

    mov edx, OFFSET uiAvailable
    mov eax, SLOTS
    sub eax, esi
    call ShowValueLine

    ; Calculate occupancy percentage
    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiOccPercent
    call WriteString

    ;for percentage (current/TOTAL) * 100
    mov eax, esi
    mov ebx, 100
    mul ebx

    mov edx, 0
    mov ebx, SLOTS
    div ebx

    push eax

    mov eax, lightGreen
    call SetTextColor

    pop eax
    call WriteDec

    mov edx, OFFSET uiPercentSign
    call WriteString
    call Crlf

    mov eax, white
    call SetTextColor

    call DisplayDivider

    ret

DisplayOccupancy ENDP

;Show parking area statistics (how many entered, how many exited, how many cars, bikes, currently parked and revenue)
DisplayStatistics PROC USES eax edx

    call DisplayDivider

    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiStatTitle
    call WriteString
    call Crlf
    call Crlf

    mov edx, OFFSET uiEntered
    mov eax, TotalEntered
    call ShowValueLine

    mov edx, OFFSET uiExited
    mov eax, TotalExited
    call ShowValueLine

    mov edx, OFFSET uiCars
    mov eax, CarsEntered
    call ShowValueLine

    mov edx, OFFSET uiBikes
    mov eax, BikesEntered
    call ShowValueLine

    call CountOccupiedSlots

    mov edx, OFFSET uiCurrent
    call ShowValueLine

    mov edx, OFFSET uiCurRevenue
    mov eax, TotalRevenue
    call ShowValueLine

    call DisplayDivider

    ret

DisplayStatistics ENDP


;revenue report (displays total revenue of bike and car and total itself)

DisplayRevenue PROC USES eax edx

    call DisplayDivider

    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiRevTitle
    call WriteString
    call Crlf
    call Crlf

    mov eax, yellow
    call SetTextColor

    mov edx, OFFSET uiRevTotal
    mov eax, TotalRevenue
    call ShowValueLine

    mov edx, OFFSET uiRevCars
    mov eax, CarRevenue
    call ShowValueLine

    mov edx, OFFSET uiRevBikes
    mov eax, BikeRevenue
    call ShowValueLine

    mov eax, white
    call SetTextColor

    call DisplayDivider

    ret

DisplayRevenue ENDP


;traffic analysis report 
DisplayTrafficAnalysis PROC USES eax ebx ecx edx esi

    call DisplayDivider

    mov eax, lightCyan
    call SetTextColor

    mov edx, OFFSET uiTrafTitle
    call WriteString
    call Crlf
    call Crlf

    ; Display all five traffic periods
    mov esi, 0

    TrafficLoop:

        cmp esi, PERIODS
        jge TrafficDone

        mov eax, lightCyan
        call SetTextColor

        mov edx, PeriodNames[esi*4]
        call WriteString

        mov edx, OFFSET uiColon
        call WriteString

        mov eax, yellow
        call SetTextColor
        mov eax, TrafficCounters[esi*4]
        call WriteDec

        mov eax, white
        call SetTextColor

        mov edx, OFFSET uiVehWord
        call WriteString
        call Crlf

        inc esi
        jmp TrafficLoop

    TrafficDone:

        call Crlf

        ; Find highest traffic count
        mov esi, 0 ;arry index
        mov ebx, 0 ;index of highest traffic period 
        mov ecx, 0 ;highest traffic  count

    PeakLoop:

        cmp esi, PERIODS
        jge PeakDone

        mov eax, TrafficCounters[esi*4]

        cmp eax, ecx
        jbe NotHigher

        mov ecx, eax
        mov ebx, esi

    NotHigher:

        inc esi
        jmp PeakLoop

    PeakDone:
        ; no traffci
        cmp ecx, 0
        je NoTraffic

        mov eax, lightGreen
        call SetTextColor

        mov edx, OFFSET uiPeak
        call WriteString
        call Crlf

        mov eax, yellow
        call SetTextColor

        mov edx, PeriodNames[ebx*4]
        call WriteString
        call Crlf

        jmp TrafficEnd

    NoTraffic:

        mov eax, yellow
        call SetTextColor

        mov edx, OFFSET uiNoTraffic
        call WriteString
        call Crlf

    TrafficEnd:

        mov eax, white
        call SetTextColor

        call DisplayDivider

        ret

DisplayTrafficAnalysis ENDP
