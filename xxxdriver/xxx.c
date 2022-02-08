#include "ntddk.h"
#include "xxx.h"

NTSTATUS DriverEntry (IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);
NTSTATUS CreateXXX (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS ReadXXX (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS WriteXXX (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS CleanupXXX (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS IoCtlXXX (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS xxx_DispatchPassThrough(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
void XXXUnload (IN PDRIVER_OBJECT DriverObject);

#define XXX_DEVICE_DEVICE_NAME (L"\\Device\\XXX")
#define XXX_DEVICE_LINK_NAME (L"\\DosDevices\\XXX")

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, CreateXXX)
#pragma alloc_text(PAGE, ReadXXX)
#pragma alloc_text(PAGE, WriteXXX)
#pragma alloc_text(PAGE, CleanupXXX)
#pragma alloc_text(PAGE, IoCtlXXX)
#pragma alloc_text(PAGE, xxx_DispatchPassThrough)
#pragma alloc_text(PAGE, XXXUnload)
#endif

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
 PDEVICE_OBJECT DeviceObject;
 UNICODE_STRING NameString, LinkString;
 NTSTATUS status;
 int i;
 struct XXX_t* xxx_ext;

 RtlInitUnicodeString(&NameString, XXX_DEVICE_DEVICE_NAME);

 status = IoCreateDevice(DriverObject, sizeof(struct XXX_t), &NameString, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
 if (!NT_SUCCESS(status)) return status;

 DeviceObject->Flags |= DO_BUFFERED_IO;

 xxx_ext = (struct XXX_t*)DeviceObject->DeviceExtension;
 RtlZeroMemory(xxx_ext, sizeof(struct XXX_t));
 xxx_ext->stringLength = 20;
 xxx_ext->stringBuf = ExAllocatePool(PagedPool, xxx_ext->stringLength);
 if (xxx_ext->stringBuf == NULL) {
	 IoDeleteDevice(DriverObject->DeviceObject);
	 return STATUS_INSUFFICIENT_RESOURCES;
 }
 RtlZeroMemory(xxx_ext->stringBuf, xxx_ext->stringLength);
 for (i = 0; i < 19; i++) xxx_ext->stringBuf[i] = i + 'A';

 RtlInitUnicodeString(&LinkString,XXX_DEVICE_LINK_NAME);
 status = IoCreateSymbolicLink(&LinkString, &NameString);
 if (!NT_SUCCESS(status)) {
  IoDeleteDevice(DriverObject->DeviceObject);
  return status;
 }

 for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
  DriverObject->MajorFunction[i] = xxx_DispatchPassThrough;
 }
 DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateXXX;
 DriverObject->MajorFunction[IRP_MJ_READ] = ReadXXX;
 DriverObject->MajorFunction[IRP_MJ_WRITE] = WriteXXX;
 DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoCtlXXX;

 DriverObject->DriverUnload = XXXUnload;

 return STATUS_SUCCESS;
}

NTSTATUS CreateXXX(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 IoCompleteRequest(Irp, IO_NO_INCREMENT);
 return STATUS_SUCCESS;
}

NTSTATUS ReadXXX(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PIO_STACK_LOCATION irpStack;
 NTSTATUS status = STATUS_SUCCESS;
 struct XXX_t* xxx_ext;
 unsigned long length = 0;

 xxx_ext = (struct XXX_t*)DeviceObject->DeviceExtension;
 irpStack = IoGetCurrentIrpStackLocation(Irp);

 if (irpStack->MajorFunction == IRP_MJ_READ) {
  length = xxx_ext->stringLength;
  if (length > irpStack->Parameters.Read.Length) length = irpStack->Parameters.Read.Length;
  if (length >= 1) {
   // Copy the buffer.
   RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, xxx_ext->stringBuf, length - 1);
   // Null terminate it.
   ((unsigned char*)Irp->AssociatedIrp.SystemBuffer)[length-1] = '\0';
  }
  Irp->IoStatus.Information = length;
 }

 IoCompleteRequest(Irp, IO_NO_INCREMENT);
 return status;
}

NTSTATUS WriteXXX(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PIO_STACK_LOCATION irpStack;
 NTSTATUS status = STATUS_SUCCESS;
 struct XXX_t* xxx_ext;
 unsigned long length = 0;
 unsigned char* buf = NULL;

 xxx_ext = (struct XXX_t*)DeviceObject->DeviceExtension;
 irpStack = IoGetCurrentIrpStackLocation(Irp);

 if (irpStack->MajorFunction == IRP_MJ_WRITE) {
	 length = irpStack->Parameters.Write.Length;
	 if (length == 0) {
		 length = 0;
		 status = STATUS_INVALID_PARAMETER;
	 }
	 else {
		buf = ExAllocatePool(PagedPool, length);
		if (buf == NULL) {
			length = 0;
			status = STATUS_INSUFFICIENT_RESOURCES;
		} else {
			ExFreePool(xxx_ext->stringBuf);
			xxx_ext->stringBuf = buf;
			xxx_ext->stringLength = length;
			RtlCopyMemory(xxx_ext->stringBuf, (unsigned char*)Irp->AssociatedIrp.SystemBuffer, xxx_ext->stringLength);
		}
	 }
 }

 Irp->IoStatus.Information = length;
 IoCompleteRequest(Irp, IO_NO_INCREMENT);
 return status;
}

NTSTATUS IoCtlXXX(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	PIO_STACK_LOCATION irpStack;
	NTSTATUS status = STATUS_SUCCESS;

	irpStack = IoGetCurrentIrpStackLocation(Irp);

	switch (irpStack->Parameters.DeviceIoControl.IoControlCode)
	{
		case IOCTL_XXX_RDTSC:
			if (irpStack->Parameters.DeviceIoControl.OutputBufferLength == 8) {
				*(unsigned long*)(Irp->AssociatedIrp.SystemBuffer) = 0x11112222;
				*((unsigned long*)Irp->AssociatedIrp.SystemBuffer+4) = 0x12345678;
				length = 8;
			} else {
				length = 0;
				status = STATUS_INVALID_PARAMETER;
			}
		break;
		default:
			status = STATUS_NOT_IMPLEMENTED;
		break;
	}

	Irp->IoStatus.Information = length;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS xxx_DispatchPassThrough(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
 PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
 NTSTATUS status = STATUS_SUCCESS;
 Irp->IoStatus.Status = status;
 IoCompleteRequest(Irp, IO_NO_INCREMENT);
 return status;
}

VOID XXXUnload(IN PDRIVER_OBJECT DriverObject)
{
 UNICODE_STRING LinkString;
 RtlInitUnicodeString(&LinkString, XXX_DEVICE_LINK_NAME);
 IoDeleteSymbolicLink(&LinkString);
 IoDeleteDevice(DriverObject->DeviceObject);
}