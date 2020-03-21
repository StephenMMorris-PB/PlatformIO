CREATE TABLE [dbo].[temhumlatlon]
(
 [Id] [bigint] IDENTITY(1,1) NOT NULL,
 [deviceID] [varchar] (12) NULL,
 [messageId] [varchar] (3) NULL,
 [temperature] [nvarchar](200) NULL,
 [humidity] [nvarchar](200) NULL,
 [latitude] [nvarchar](200) NULL,
 [longitude] [nvarchar](200) NULL,
 [EventProcessedUtcTime] [datetime] NULL,
 [PartitionId] [varchar] (3) NULL,
 [EventEnqueuedUtcTime] [datetime] NULL,
 [IoTHub] [nvarchar](200) NULL,
 CONSTRAINT [PK_temhumlatlon] PRIMARY KEY CLUSTERED
 (
  [id] ASC
 )WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]