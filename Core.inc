.data
;======CONSTANTS======
SLOTS = 32
PERIODS = 5 ; FOR TRAFFICS
CAR  = 1
BIKE = 2

; ===== PARKING FEE CONSTANTS =====
; first hour fee and extra hours fees
CAR_FIRST_FEE = 50
CAR_EXTRA_FEE = 30
BIKE_FIRST_FEE = 20
BIKE_EXTRA_FEE = 10

;======PARKING DATA ======
;A bit represents a parking slot (0 for vacant and 1 for occupied)
OccupancyMask DWORD 0

; ======VEHICLE RECORD ARRAYS ======
; Each array index represents a parking slot.
VehicleIDs DWORD SLOTS DUP(0)
VehicleTypes BYTE SLOTS DUP(0)
EntryHours DWORD SLOTS DUP(0)
RecordActive BYTE SLOTS DUP(0)

; ===== VEHICLE ENTRY INPUTS =====
InputVehicleID DWORD 0
InputVehicleType BYTE 0
InputEntryHour DWORD 0

; ===== VEHICLE EXIT INPUTS =====
InputExitHour DWORD 0
ParkingDuration DWORD 0
ParkingFee DWORD 0

; ====== PARKING STATISTICS ======
CurrentVehicleCount DWORD 0
TotalRevenue DWORD 0

; ====== TRAFFIC COUNTERS ======
; Convention
; Index 0 = 07:00 - 9:00
; Index 1 = 9:00 - 11:00
; Index 2 = 11:00 - 13:00
; Index 3 = 13:00 - 15:00
; Index 4 = 15:00 - 17:00
TrafficCounters DWORD PERIODS DUP(0)

; ===== VEHICLE ENTRY MESSAGES =====
msgEnterID BYTE "Enter vehicle ID: ", 0
msgEnterType BYTE "Vehicle type (1 = Car, 2 = Bike): ", 0
msgEnterHour BYTE "Enter entry hour (0-23): ", 0

msgDuplicate BYTE "Error: Vehicle ID is already registered.", 0
msgParkingFull BYTE "Error: Parking area is full.", 0
msgInvalidType BYTE "Error: Invalid vehicle type.", 0
msgInvalidHour BYTE "Error: Invalid entry hour.", 0
msgInvalidID BYTE "Error: Invalid Vehicle ID Entered.", 0

msgEntrySuccess BYTE "Vehicle registered successfully!", 0
msgAssignedSlot BYTE "Assigned parking slot: ", 0

; ===== VEHICLE Search MESSAGES =====
msgSearchID BYTE "Enter vehicle ID to search: ", 0
msgVehicleID BYTE "Vehicle ID: ", 0
msgVehicleType BYTE "Vehicle Type: ", 0
msgVehicleSlot BYTE "Assigned Slot: ", 0
msgVehicleHour BYTE "Entry Hour: ", 0

msgCar BYTE "Car", 0
msgBike BYTE "Bike", 0

msgVehicleFound BYTE "Vehicle found!", 0
msgVehicleNotFound BYTE "Vehicle not found.", 0

; ===== VEHICLE EXIT MESSAGES =====
msgExitID BYTE "Enter vehicle ID to exit: ", 0
msgExitHour BYTE "Enter exit hour (0-23): ", 0
msgExitSuccess BYTE "Vehicle exited successfully!", 0

msgDuration BYTE "Parking duration (hours): ", 0
msgFee BYTE "Parking fee: PKR ", 0
msgRevenue BYTE "Total revenue: PKR ", 0

msgExitNotFound BYTE "Error: Vehicle is not currently parked.", 0
msgInvalidExitHour BYTE "Error: Invalid exit hour.", 0

; ===== TRAFFIC REPORT MESSAGES =====
msgTrafficTitle BYTE "========== TRAFFIC REPORT ==========", 0
msgPeriod0 BYTE "07:00 - 09:00: ", 0
msgPeriod1 BYTE "09:00 - 11:00: ", 0
msgPeriod2 BYTE "11:00 - 13:00: ", 0
msgPeriod3 BYTE "13:00 - 15:00: ", 0
msgPeriod4 BYTE "15:00 - 17:00: ", 0

msgVehicles BYTE " vehicles entered", 0

msgNotTracked BYTE "Entry hour is outside monitored periods.", 0

;============ CODE ============
.code

CheckSlot PROC USES ebx ecx
    COMMENT!
    It checks whether a certain slot is occupied or empty
    We use TEST instruction
    If ZF = 0 then slot is occupied 
    if ZF = 1 then slot is empty
    
    Requires: EAX = slot index
    Returns: EAX 
    !

    ;Invalid Checks (if slots<0 || slots >= 32)
    cmp eax , 0
    jl InvalidCheck
    cmp eax, SLOTS
    jge InvalidCheck

    ;We shift the ebx = 1 to the required SLOT bit so we can test it
    ;we took that 1 to that slot by left shifting
    mov ecx, eax
    mov ebx,1
    shl ebx, cl

    ;we now check for that slot occupied or empty 
    mov eax, OccupancyMask
    test eax, ebx

    jz EmptySlot

    ;if not empty 
    mov eax , 1
    jmp CheckDone

    EmptySlot:
        mov eax,0 ;if empty

    CheckDone:
        ret

    InvalidCheck:
        mov eax, -1
        ret
CheckSlot ENDP

OccupySlot PROC USES ebx ecx
    COMMENT!
    To mark the slot as occupied

    we use OR , and to set the bit we OR with 1 at that bit

    Requires: EAX = Slot Index
    Returns: EAX= 1 if sucess and EAX = 0 if invalid
    !

    ;Invalid Check ( index <0 || index >= 32)
    cmp eax,0
    jl InvalidOccupy

    cmp eax, SLOTS
    jge InvalidOccupy

    ;now we need the mask that contains 1 at that index 
    ;we put 1 to that slot by left shifting it
    mov ecx, eax
    mov ebx, 1
    shl ebx, cl

    OR OccupancyMask, ebx

    ;success
    mov eax, 1
    ret

    InvalidOccupy:
        mov eax,0
        ret
OccupySlot ENDP

ReleaseSlot PROC USES ebx ecx
    COMMENT!
    To mark slot as available 

    We AND the occupancyMask with that bits that CLEARS That certain Slot ONLY and keep the rest UNCHANGED.

    requires: EAX = SLOT INDEX
    returns: EAX = 1 if sucess and EAX = 0 if invalid
    !

    ;INVALID Boundary
    cmp eax, 0
    jl InvalidRelease

    cmp eax, SLOTS
    jge InvalidRelease

    mov ecx, eax
    mov ebx, 1
    shl ebx, cl
    not ebx  ;we invert that slot to clear that slot
    and OccupancyMask, ebx
    
    mov eax, 1
    ret

    InvalidRelease:
        mov eax, 0
        ret

ReleaseSlot ENDP

FindAvailableSlot PROC USES esi

    COMMENT!
    To find first available slot


    returns: EAX = index of first availble slot, EAX = -1 for full 
    !

    mov esi, 0

    SearchLoop:
        ; all slots traversed
        cmp esi, SLOTS
        jge ParkingFull

        ;slot index
        mov eax, esi
        call CheckSlot  ;check slot if occupied or not

        ;if 0 then it is empty
        cmp eax, 0
        je AvailableFound

        inc esi
        jmp SearchLoop

    AvailableFound:
        mov eax, esi ;available slot index
        ret

    ParkingFull:
        ;no available slot found
        mov eax, -1
        ret

FindAvailableSlot ENDP


VehicleEntry PROC USES esi

    COMMENT!
    Registers a new vehicle in the parking system.

    Input:
    Vehicle ID, vehicle type, and entry hour
    are accepted from the user.

    Returns:
    EAX = 1 if registration succeeds
    EAX = 0 if registration fails
    !

    ; =================Input of Vehicle ID=================

    mov edx, OFFSET msgEnterID
    call WriteString
    call ReadDec

    mov InputVehicleID, eax

    ; Vehicle ID cannot be zero

    cmp eax, 0
    je InvalidID

    ;=================CHECK FOR DUPLICATE VEHICLE ID=================
    mov esi, 0

    DuplicateLoop:
        cmp esi, SLOTS
        jge DuplicateCheckDone

        ; Only check active records

        cmp RecordActive[esi], 1
        jne NextRecord

        mov eax, InputVehicleID

        ;esi * 4 to go ahead of array
        cmp VehicleIDs[esi*4], eax
        je DuplicateFound

    NextRecord:
        inc esi
        jmp DuplicateLoop

    DuplicateCheckDone:
        ; ================= Input VEHICLE TYPE================
        mov edx, OFFSET msgEnterType
        call WriteString
        call ReadDec

        ;CAR IS FOR 1
        cmp eax, CAR
        je ValidType

        ;Bike is for 2
        cmp eax, BIKE
        je ValidType

        jmp InvalidType

    ValidType:
        mov InputVehicleType, al
        ; ================= Input ENTRY HOUR=================
        mov edx, OFFSET msgEnterHour
        call WriteString
        call ReadDec

        cmp eax, 23
        ja InvalidHour

        mov InputEntryHour, eax

        ;================= FIND AVAILABLE PARKING SLOT =================

        call FindAvailableSlot

        cmp eax, -1
        je ParkingFull

        ; EAX contains the available slot index.

        mov esi, eax

        ;=================STORE VEHICLE INFORMATION IN ARRAY=================

        mov eax, InputVehicleID
        mov VehicleIDs[esi*4], eax

        mov al, InputVehicleType
        mov VehicleTypes[esi], al

        mov eax, InputEntryHour
        mov EntryHours[esi*4], eax

        mov RecordActive[esi], 1

        ; ================= MARK SLOT AS OCCUPIED =================
        mov eax, esi
        call OccupySlot

        ;=================UPDATE VEHICLE COUNT=================
        inc CurrentVehicleCount

        ; ;=================Update traffic counter=================
        mov eax, InputEntryHour
        call UpdateTrafficCounter

        cmp eax, 0
        jne TrafficUpdateDone

        mov edx, OFFSET msgNotTracked
        call WriteString
        call Crlf

        TrafficUpdateDone:
            ; ================= DISPLAY SUCCESS MESSAGE =================
            mov edx, OFFSET msgEntrySuccess
            call WriteString
            call Crlf

            mov edx, OFFSET msgAssignedSlot
            call WriteString

            ; Display actual slot number (1-32) (not from 0th index)
            mov eax, esi
            inc eax
            call WriteDec
            call Crlf

            mov eax, 1
            ret

    ; =================ERROR HANDLING=================

    DuplicateFound:

        mov edx, OFFSET msgDuplicate
        call WriteString
        call Crlf

        mov eax, 0
        ret

    ParkingFull:

        mov edx, OFFSET msgParkingFull
        call WriteString
        call Crlf

        mov eax, 0
        ret

    InvalidType:

        mov edx, OFFSET msgInvalidType
        call WriteString
        call Crlf

        mov eax, 0
        ret

    InvalidHour:

        mov edx, OFFSET msgInvalidHour
        call WriteString
        call Crlf

        mov eax, 0
        ret

    InvalidID:

        mov edx, OFFSET msgInvalidID
        call WriteString
        call Crlf

        mov eax, 0
        ret

VehicleEntry ENDP


SearchVehicle PROC USES esi

    COMMENT!
    Searches for a registered vehicle using its ID.

     linear search through the vehicle arrays.

    Returns:
    EAX = 1 if vehicle is found
    EAX = 0 if vehicle is not found
    !

    ; =====================INPUT VEHICLE ID=====================
    mov edx, OFFSET msgSearchID
    call WriteString
    call ReadDec

    mov InputVehicleID, eax

    ;=====================INITIALIZE SEARCH INDEX=====================
    mov esi, 0

    SearchLoop:
        cmp esi, SLOTS
        jge VehicleNotFound

        ; ===================== CHECK WHETHER RECORD IS ACTIVE=====================
        cmp RecordActive[esi], 1
        jne NextSearchRecord

        ; ===================== COMPARE VEHICLE IDs=================================
        mov eax, InputVehicleID
        cmp VehicleIDs[esi*4], eax
        je VehicleFound
         
    NextSearchRecord:

        inc esi
        jmp SearchLoop

       
    ; ===================== DISPLAY FOUND VEHICLE=====================
    VehicleFound:
        mov edx, OFFSET msgVehicleFound
        call WriteString
        call Crlf

        ; Display vehicle ID
        mov edx, OFFSET msgVehicleID
        call WriteString

        mov eax, VehicleIDs[esi*4]
        call WriteDec
        call Crlf

        ; Display vehicle type
        mov edx, OFFSET msgVehicleType
        call WriteString

        mov al, VehicleTypes[esi]

        cmp al, CAR
        je DisplayCar

        cmp al, BIKE
        je DisplayBike

        jmp DisplaySlot

    DisplayCar:
        mov edx, OFFSET msgCar
        call WriteString
        call Crlf
        jmp DisplaySlot

    DisplayBike:
        mov edx, OFFSET msgBike
        call WriteString
        call Crlf

        ; Display assigned parking slot

    DisplaySlot:
        mov edx, OFFSET msgVehicleSlot
        call WriteString

        mov eax, esi
        inc eax
        call WriteDec
        call Crlf

        ; Display entry hour
        mov edx, OFFSET msgVehicleHour
        call WriteString

        mov eax, EntryHours[esi*4]
        call WriteDec
        call Crlf

        mov eax, 1
        ret

    ; ================================== VEHICLE NOT FOUND==================================
       
    VehicleNotFound:
        mov edx, OFFSET msgVehicleNotFound
        call WriteString
        call Crlf

        mov eax, 0
        ret

SearchVehicle ENDP


VehicleExit PROC USES esi ebx ecx

    COMMENT!
    Finds an active vehicle, calculates its parking fees,releases its parking slot, and updates parking records.

    Returns:
    EAX = 1 if exit successful
    EAX = 0 if exit unsuccessful
    !

    ; ===================== ACCEPT VEHICLE ID=====================
    mov edx, OFFSET msgExitID
    call WriteString
    call ReadDec

    mov InputVehicleID, eax

    ; ===================== FIND ACTIVE VEHICLE=====================
    mov esi, 0

    ExitSearchLoop:
        cmp esi, SLOTS
        jge ExitVehicleNotFound

        cmp RecordActive[esi], 1
        jne NextExitRecord

        mov eax, InputVehicleID
        cmp VehicleIDs[esi*4], eax
        je ExitVehicleFound

    NextExitRecord:
        inc esi
        jmp ExitSearchLoop

     
    ; ===================== INPUT EXIT HOUR=====================
    ExitVehicleFound:

        mov edx, OFFSET msgExitHour
        call WriteString
        call ReadDec

        cmp eax, 23
        ja InvalidExitHour

        mov InputExitHour, eax

        ;CALCULATE PARKING DURATION

        mov eax, InputExitHour
        cmp eax, EntryHours[esi*4]
        jb CrossedMidnight

        ;IF  exit hour >= entry hour
        ; Duration = ExitHour - EntryHour
        sub eax, EntryHours[esi*4]
        jmp DurationCalculated

    CrossedMidnight:
        ;if (exit hour < entry hour)
        ; Midnight case:
        ; Duration = (24 - EntryHour) + ExitHour

        mov eax, 24
        sub eax, EntryHours[esi*4]
        add eax, InputExitHour

    DurationCalculated:
        ; Same entry and exit hour means 24 hours
        cmp eax, 0
        jne StoreDuration

        mov eax, 24

    StoreDuration:

        mov ParkingDuration, eax
        ;===================== CALCULATE PARKING FEE=====================
        ; Determine vehicle type
        mov al, VehicleTypes[esi]

        cmp al, CAR
        je CalculateCarFee

        ; Otherwise, use Bike 

    CalculateBikeFee:

        mov eax, BIKE_FIRST_FEE
        mov ebx, BIKE_EXTRA_FEE
        jmp CalculateAdditionalFee

    CalculateCarFee:

        mov eax, CAR_FIRST_FEE
        mov ebx, CAR_EXTRA_FEE

    CalculateAdditionalFee:

        ; EAX = first hour fee
        ; EBX = additional hours fee
        ; ECX = number of additional hours

        mov ecx, ParkingDuration
        dec ecx ;according to 0th index

        cmp ecx, 0
        je FeeCalculated

        ;THE FORMULA IS
        ; FEES =  first hour fee  + (duration -1) * additional hour fee
    FeeLoop:
        add eax, ebx
        loop FeeLoop

    FeeCalculated:
        mov ParkingFee, eax

        ;===================== UPDATE TOTAL REVENUE =====================
        add TotalRevenue, eax

        ; ===================== RELEASE PARKING =====================
        mov eax, esi
        call ReleaseSlot

        ; ===================== MARK VEHICLE AS INACTIVE =====================
        mov RecordActive[esi], 0

        ; ================================== UPDATE VEHICLE COUNT ==================================
        dec CurrentVehicleCount

        ; ================================== DISPLAY EXIT ==================================
         mov edx, OFFSET msgExitSuccess
        call WriteString
        call Crlf

        mov edx, OFFSET msgDuration
        call WriteString

        mov eax, ParkingDuration
        call WriteDec
        call Crlf

        mov edx, OFFSET msgFee
        call WriteString

        mov eax, ParkingFee
        call WriteDec
        call Crlf

        mov edx, OFFSET msgRevenue
        call WriteString

        mov eax, TotalRevenue
        call WriteDec
        call Crlf

        mov eax, 1
        ret
   
     ; ==================================ERROR HANDLING==================================
    InvalidExitHour:
        mov edx, OFFSET msgInvalidExitHour
        call WriteString
        call Crlf

        mov eax, 0
        ret

    ExitVehicleNotFound:
        mov edx, OFFSET msgExitNotFound
        call WriteString
        call Crlf

        mov eax, 0
        ret

VehicleExit ENDP


UpdateTrafficCounter PROC USES ebx

    COMMENT!
    Determines the traffic period from the entry hour and increments the corresponding traffic counter.

    Input:
        EAX = vehicle entry hour (0-23)

    Returns:
        EAX = 1 if a counter was updated
        EAX = 0 if hour is outside monitored periods
    !

    ;We use fall through logic

    ; 07:00 - 9:00 => index 0

    cmp eax, 7
    jb NotMonitored

    cmp eax, 9
    jb Period0

    ; 9:00 - 11:00 => index 1

    cmp eax, 11
    jb Period1

    ; 11:00 - 13:00 => index 2

    cmp eax, 13
    jb Period2

    ; 13:00 - 15:00 => index 3

    cmp eax, 15
    jb Period3

    ; 15:00 - 17:00 => index 4

    cmp eax, 17
    jb Period4

    ;if not in the monitored records
    jmp NotMonitored

    Period0:
        mov ebx, 0
        jmp IncrementCounter

    Period1:
        mov ebx, 1
        jmp IncrementCounter

    Period2:
        mov ebx, 2
        jmp IncrementCounter

    Period3:
        mov ebx, 3
        jmp IncrementCounter

    Period4:
        mov ebx, 4

    IncrementCounter:
        ;EBX contains the index of the period
        inc TrafficCounters[ebx*4]

        mov eax, 1
        ret

    NotMonitored:

        mov eax, 0
        ret

UpdateTrafficCounter ENDP


DisplayTrafficReport PROC
    mov edx, OFFSET msgTrafficTitle
    call WriteString
    call Crlf

    ; Period 0
    mov edx, OFFSET msgPeriod0
    call WriteString

    mov eax, TrafficCounters[0]
    call WriteDec

    mov edx, OFFSET msgVehicles
    call WriteString
    call Crlf

    ; Period 1

    mov edx, OFFSET msgPeriod1
    call WriteString

    mov eax, TrafficCounters[4]
    call WriteDec

    mov edx, OFFSET msgVehicles
    call WriteString
    call Crlf

    ; Period 2

    mov edx, OFFSET msgPeriod2
    call WriteString

    mov eax, TrafficCounters[8]
    call WriteDec

    mov edx, OFFSET msgVehicles
    call WriteString
    call Crlf

    ; Period 3

    mov edx, OFFSET msgPeriod3
    call WriteString

    mov eax, TrafficCounters[12]
    call WriteDec

    mov edx, OFFSET msgVehicles
    call WriteString
    call Crlf

    ; Period 4

    mov edx, OFFSET msgPeriod4
    call WriteString

    mov eax, TrafficCounters[16]
    call WriteDec

    mov edx, OFFSET msgVehicles
    call WriteString
    call Crlf

    ret

DisplayTrafficReport ENDP
