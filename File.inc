
.data

; ================= DATA =================
SaveFileName BYTE "SmartPark.dat", 0

FileHandle DWORD ?
FileBytes  DWORD ?

FileMagic   DWORD 53504B31h
FileVersion DWORD 1

; ================= FILE HANDLING PROCEDURES =================

.code
WriteBlock PROC
    ; --------------------------------------------------------
    ; WriteBlock
    ; EDX = address of data
    ; ECX = number of bytes
    ; Returns EAX = 1 if successful, 0 otherwise
    ; --------------------------------------------------------

    mov FileBytes, ecx
    mov eax, FileHandle

    call WriteToFile

    cmp eax, FileBytes
    jne WriteBlockFailed

    mov eax, 1
    ret

    WriteBlockFailed:
        mov eax, 0
        ret

WriteBlock ENDP



ReadBlock PROC
    ; --------------------------------------------------------
    ; ReadBlock
    ; EDX = address of destination
    ; ECX = number of bytes
    ; Returns EAX = 1 if successful, 0 otherwise
    ; --------------------------------------------------------

    mov FileBytes, ecx
    mov eax, FileHandle

    call ReadFromFile

    cmp eax, FileBytes
    jne ReadBlockFailed

    mov eax, 1
    ret

    ReadBlockFailed:
        mov eax, 0
        ret

ReadBlock ENDP



ResetParkingData PROC USES eax ecx edi
    ; --------------------------------------------------------
    ; ResetParkingData
    ; Resets all saved parking information.
    ; --------------------------------------------------------

    mov OccupancyMask, 0

    ; Reset vehicle IDs
    mov edi, OFFSET VehicleIDs
    mov ecx, LENGTHOF VehicleIDs
    xor eax, eax
    cld
    rep stosd

    ; Reset vehicle types
    mov edi, OFFSET VehicleTypes
    mov ecx, SIZEOF VehicleTypes
    xor eax, eax
    rep stosb

    ; Reset entry hours
    mov edi, OFFSET EntryHours
    mov ecx, LENGTHOF EntryHours
    xor eax, eax
    rep stosd

    ; Reset active records
    mov edi, OFFSET RecordActive
    mov ecx, SIZEOF RecordActive
    xor eax, eax
    rep stosb

    ; Reset traffic counters
    mov edi, OFFSET TrafficCounters
    mov ecx, LENGTHOF TrafficCounters
    xor eax, eax
    rep stosd

    ; Reset parking statistics
    mov CurrentVehicleCount, 0
    mov TotalRevenue, 0

    ; Reset UI statistics
    mov TotalEntered, 0
    mov TotalExited, 0
    mov CarsEntered, 0
    mov BikesEntered, 0
    mov CarRevenue, 0
    mov BikeRevenue, 0

    ret

ResetParkingData ENDP



SaveParkingData PROC USES edx ecx
    ; --------------------------------------------------------
    ; SaveParkingData
    ; Saves all important parking information to SmartPark.dat
    ; Returns EAX = 1 if successful, 0 otherwise.
    ; --------------------------------------------------------

    ; Create output file
    mov edx, OFFSET SaveFileName
    call CreateOutputFile

    cmp eax, -1
    je SaveFailed

    mov FileHandle, eax

    ; Save file signature
    mov edx, OFFSET FileMagic
    mov ecx, TYPE FileMagic
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save file version
    mov edx, OFFSET FileVersion
    mov ecx, TYPE FileVersion
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save occupancy mask
    mov edx, OFFSET OccupancyMask
    mov ecx, TYPE OccupancyMask
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save vehicle IDs
    mov edx, OFFSET VehicleIDs
    mov ecx, SIZEOF VehicleIDs
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save vehicle types
    mov edx, OFFSET VehicleTypes
    mov ecx, SIZEOF VehicleTypes
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save entry hours
    mov edx, OFFSET EntryHours
    mov ecx, SIZEOF EntryHours
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save active records
    mov edx, OFFSET RecordActive
    mov ecx, SIZEOF RecordActive
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save current vehicle count
    mov edx, OFFSET CurrentVehicleCount
    mov ecx, TYPE CurrentVehicleCount
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save total revenue
    mov edx, OFFSET TotalRevenue
    mov ecx, TYPE TotalRevenue
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save traffic counters
    mov edx, OFFSET TrafficCounters
    mov ecx, SIZEOF TrafficCounters
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    ; Save reporting counters
    mov edx, OFFSET TotalEntered
    mov ecx, TYPE TotalEntered
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    mov edx, OFFSET TotalExited
    mov ecx, TYPE TotalExited
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    mov edx, OFFSET CarsEntered
    mov ecx, TYPE CarsEntered
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    mov edx, OFFSET BikesEntered
    mov ecx, TYPE BikesEntered
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    mov edx, OFFSET CarRevenue
    mov ecx, TYPE CarRevenue
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    mov edx, OFFSET BikeRevenue
    mov ecx, TYPE BikeRevenue
    call WriteBlock
    cmp eax, 1
    jne SaveFailed

    SaveSuccess:

        mov eax, FileHandle
        call CloseFile

        mov eax, 1
        ret

    SaveFailed:

        mov eax, FileHandle
        call CloseFile

        xor eax, eax
        ret

SaveParkingData ENDP



LoadParkingData PROC USES edx ecx
    ; --------------------------------------------------------
    ; LoadParkingData
    ; Loads previously saved parking information.
    ; If no valid file exists, the program uses initial data.
    ; Returns EAX = 1 if successful, 0 otherwise.
    ; --------------------------------------------------------

    ; Open existing file
    mov edx, OFFSET SaveFileName
    call OpenInputFile

    cmp eax, -1
    je LoadFailed

    mov FileHandle, eax

    ; Validate file signature
    mov edx, OFFSET FileMagic
    mov ecx, TYPE FileMagic
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    cmp FileMagic, 53504B31h
    jne InvalidSaveFile

    ; Validate file version
    mov edx, OFFSET FileVersion
    mov ecx, TYPE FileVersion
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    cmp FileVersion, 1
    jne InvalidSaveFile

    ; Load occupancy mask
    mov edx, OFFSET OccupancyMask
    mov ecx, TYPE OccupancyMask
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load vehicle IDs
    mov edx, OFFSET VehicleIDs
    mov ecx, SIZEOF VehicleIDs
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load vehicle types
    mov edx, OFFSET VehicleTypes
    mov ecx, SIZEOF VehicleTypes
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load entry hours
    mov edx, OFFSET EntryHours
    mov ecx, SIZEOF EntryHours
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load active records
    mov edx, OFFSET RecordActive
    mov ecx, SIZEOF RecordActive
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load current vehicle count
    mov edx, OFFSET CurrentVehicleCount
    mov ecx, TYPE CurrentVehicleCount
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load total revenue
    mov edx, OFFSET TotalRevenue
    mov ecx, TYPE TotalRevenue
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load traffic counters
    mov edx, OFFSET TrafficCounters
    mov ecx, SIZEOF TrafficCounters
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    ; Load reporting counters
    mov edx, OFFSET TotalEntered
    mov ecx, TYPE TotalEntered
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    mov edx, OFFSET TotalExited
    mov ecx, TYPE TotalExited
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    mov edx, OFFSET CarsEntered
    mov ecx, TYPE CarsEntered
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    mov edx, OFFSET BikesEntered
    mov ecx, TYPE BikesEntered
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    mov edx, OFFSET CarRevenue
    mov ecx, TYPE CarRevenue
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    mov edx, OFFSET BikeRevenue
    mov ecx, TYPE BikeRevenue
    call ReadBlock
    cmp eax, 1
    jne InvalidSaveFile

    LoadSuccess:
        mov eax, FileHandle
        call CloseFile

        mov eax, 1
        ret

    InvalidSaveFile:
        mov eax, FileHandle
        call CloseFile

        call ResetParkingData

    LoadFailed:

        xor eax, eax
        ret

LoadParkingData ENDP
